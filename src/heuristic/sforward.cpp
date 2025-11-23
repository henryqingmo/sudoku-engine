#include <algorithm>
#include <heuristic/sforward.h>

using sudoku_engine::SForwardHeuristic;

void SForwardHeuristic::startSteppingForward() {
    forward_stack.clear();
    forward_stepping_initialized = true;

    // Determine initial starting position: if MRV active, findMrvCell() else {0,0}
    if (this->mrv) {
        BoardPosition start = this->findMrvCell();
        // If board already full, create a frame with row==BOARD_SIZE so step() returns true immediately.
        forward_stack.push_back(
            ForwardFrame{ForwardFrame::Init, start, false, {}, 0, {}, false}
        );
    } else {
        forward_stack.push_back(
            ForwardFrame{
                ForwardFrame::Init, BoardPosition{0, 0}, false, {}, 0, {}, false
            }
        );
    }
}

SForwardHeuristic::StepResult SForwardHeuristic::step() {
    if (!forward_stepping_initialized) {
        startSteppingForward();
    }

    if (forward_stack.empty()) {
        // No frames -> unsolvable / finished without success
        return StepResult::STEP_FAIL;
    }

    ForwardFrame& f = forward_stack.back();
    const BoardPosition pos = f.pos;

    // If this frame represents the terminal condition (board full), we found a solution.
    if (pos.row == BOARD_SIZE) {
        return StepResult::STEP_SUCC;
    }

    // If we are returning to this frame after a child was pushed and failed,
    // then f.state == TryingCandidate and has_current_unrefined == true.
    // In that case, we must undo the domain deltas and clear the cell, then advance to next candidate.
    if (f.state == ForwardFrame::TryingCandidate && f.has_current_unrefined) {
        // child returned failure -> backtrack
        // restore board cell and domains for this candidate
        auto& cell_value = this->board.getValues()[pos];
        cell_value = CELL_EMPTY;

        // restore domain deltas
        this->applyDeltas(std::move(f.current_unrefined));
        f.has_current_unrefined = false;
        f.try_index++;
        // move back into GeneratedCandidates state (so next step will try next candidate or pop)
        f.state = ForwardFrame::GeneratedCandidates;

        // Do only the restore in this step — let the next call try the next candidate.
        return StepResult::STEP_MORE;
    }

    // If we're just entering this frame for the first time, generate candidates or skip cell.
    if (f.state == ForwardFrame::Init) {
        this->step_count++;
        this->checkStepLimit();

        auto& cell_value = this->board.getValues()[pos];

        // If the cell is already filled, emulate recursive "expand(next_pos)"
        if (cell_value != CELL_EMPTY) {
            f.skip_cell = true;
            // Determine next position according to MRV or simple increment
            BoardPosition child_pos;
            if (this->mrv) {
                child_pos = this->findMrvCell();
                // If board is full already, push terminal frame so next step returns true
                if (child_pos.row >= BOARD_SIZE) {
                    forward_stack.push_back(
                        ForwardFrame{
                            ForwardFrame::Init,
                            child_pos,
                            false,
                            {},
                            0,
                            {},
                            false
                        }
                    );
                    return StepResult::STEP_MORE;
                }
            } else {
                // incrementPos from current pos as recursive does
                child_pos = this->incrementPos(pos);
                if (child_pos.row == BOARD_SIZE) {
                    // child is terminal
                    forward_stack.push_back(
                        ForwardFrame{
                            ForwardFrame::Init,
                            child_pos,
                            false,
                            {},
                            0,
                            {},
                            false
                        }
                    );
                    return StepResult::STEP_MORE;
                }
            }

            // push child frame and return; child frame will be processed in the next step
            forward_stack.push_back(
                ForwardFrame{
                    ForwardFrame::Init, child_pos, false, {}, 0, {}, false
                }
            );
            return StepResult::STEP_MORE;
        }

        // Otherwise, cell is empty: build list of legal candidates with forward-check refinement
        f.candidates.clear();
        for (BoardCell num = CELL_MIN; num <= CELL_MAX; ++num) {
            cell_value = num;

            // if number not in domain or invalid placement, skip
            if (!this->cell_domains[pos].has(num) ||
                this->board.isInvalid(pos)) {
                cell_value = CELL_EMPTY;
                continue;
            }

            auto refinement = this->forwardCheck(pos);
            if (!refinement.is_legal) {
                // leave cell empty and continue
                cell_value = CELL_EMPTY;
                continue;
            }

            // store candidate and its refinement (we will apply refinement when we actually try it)
            f.candidates.emplace_back(num, std::move(refinement));
        }

        // Restore the cell to empty as recursive expand did
        cell_value = CELL_EMPTY;

        // Apply LCV ordering if requested
        if (this->lcv && !f.candidates.empty()) {
            std::sort(
                f.candidates.begin(),
                f.candidates.end(),
                [](const std::pair<BoardCell, RefinedDomains>& a,
                   const std::pair<BoardCell, RefinedDomains>& b) -> bool {
                    return a.second.values_pruned < b.second.values_pruned;
                }
            );
        }

        f.try_index = 0;
        f.state = ForwardFrame::GeneratedCandidates;

        // If no candidates, the next step will pop this frame (i.e., return false up the stack)
        // But to emulate the recursive call, return false now and let subsequent step handle popping if needed.
        return StepResult::STEP_MORE;
    }

    // Now f.state == GeneratedCandidates: we should attempt the next candidate (or pop if none).
    if (f.state == ForwardFrame::GeneratedCandidates) {
        // No candidates -> failure for this frame: pop and return (backtracks to parent)
        if (f.try_index >= f.candidates.size()) {
            // ensure cell is empty (safety)
            this->board.getValues()[pos] = CELL_EMPTY;
            forward_stack.pop_back();
            return StepResult::STEP_MORE;
        }

        // Otherwise, try the next candidate: set cell, apply deltas with backup, then push child frame.
        auto& candidate = f.candidates[f.try_index];
        BoardCell num = candidate.first;
        RefinedDomains refinement = std::move(candidate.second);

        // Place the number
        this->board.getValues()[pos] = num;

        // Apply refinement deltas and keep the unrefined backup for backtracking
        DomainDeltas unrefined =
            this->applyDeltasWithBackup(std::move(refinement.new_domains));
        f.current_unrefined = std::move(unrefined);
        f.has_current_unrefined = true;

        // Mark that we are currently trying this candidate; when child returns, we'll detect and restore.
        f.state = ForwardFrame::TryingCandidate;

        // Determine child's position (MRV recalculated in child frame, or increment for non-MRV)
        BoardPosition child_pos;
        if (this->mrv) {
            child_pos = this->findMrvCell();
            // If board full now (child_pos row >= BOARD_SIZE), push terminal so next step returns true
            if (child_pos.row >= BOARD_SIZE) {
                forward_stack.push_back(
                    ForwardFrame{
                        ForwardFrame::Init, child_pos, false, {}, 0, {}, false
                    }
                );
                return StepResult::STEP_MORE;
            }
        } else {
            child_pos = this->incrementPos(pos);
            if (child_pos.row == BOARD_SIZE) {
                forward_stack.push_back(
                    ForwardFrame{
                        ForwardFrame::Init, child_pos, false, {}, 0, {}, false
                    }
                );
                return StepResult::STEP_MORE;
            }
        }

        // Push the child's frame and return; the child frame's processing (and step_count++) will
        // occur on the next call to step().
        forward_stack.push_back(
            ForwardFrame{ForwardFrame::Init, child_pos, false, {}, 0, {}, false}
        );
        return StepResult::STEP_MORE;
    }

    // Fallback: should not reach here, but handle gracefully by popping frame.
    forward_stack.pop_back();
    return StepResult::STEP_FAIL;
}
