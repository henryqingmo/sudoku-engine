#include "heuristic/forward.h"

using sudoku_engine::BoardCellDomain;
using sudoku_engine::ForwardHeuristic;

ForwardHeuristic::ForwardHeuristic(Board& board)
    : BacktrackHeuristic(board), cell_domains(BOARD_SIZE, BoardCellDomain{1, 2, 3, 4, 5, 6, 7, 8, 9}) {
    BoardPosition pos = {0, 0};
    for (pos.row = 0; pos.row < BOARD_SIZE; pos.row++) {
        for (pos.col = 0; pos.col < BOARD_SIZE; pos.col++) {
            if (auto val = this->board.getValues()[pos]; val != CELL_EMPTY) {
                this->cell_domains[pos] = {val};
            }
        }
    }
}

BoardCellDomain ForwardHeuristic::getInvalidChoices(
    const BoardPosition& pos
) const {
    return -this->cell_domains[pos];
}

bool ForwardHeuristic::onUpdate(const BoardPosition& pos) {
    const BoardCell new_value = this->board.getValues()[pos];

    // Reset domains when backtracking...
    if (new_value == CELL_EMPTY) {
        if (this->deltas.empty()) {
            // Nothing to restore
            return true;
        }
        
        for (auto& [p, domain] : this->deltas.top()) {
            this->cell_domains[p] = std::move(domain);
        }

        this->deltas.pop();
        return true;
    }

    this->deltas.push({{pos, this->cell_domains[pos]}});
    this->cell_domains[pos] = {new_value};

    DomainDeltas& iteration_deltas = this->deltas.top();
    
    // Track which positions we've already refined to avoid duplicates
    std::bitset<BOARD_SIZE * BOARD_SIZE> refined;

    const auto refine_domain = [&](const BoardPosition& p) -> bool {
        // Skip already-filled cells - their domains shouldn't be modified
        if (this->board.getValues()[p] != CELL_EMPTY) {
            return true;
        }
        
        // Skip if we've already refined this cell
        std::size_t p_index = p.row * BOARD_SIZE + p.col;
        if (refined.test(p_index)) {
            return true;
        }
        refined.set(p_index);
        
        // Save the domain BEFORE modifying it
        BoardCellDomain old_domain = this->cell_domains[p];
        this->cell_domains[p].remove(new_value);
        iteration_deltas.push_back({p, old_domain});
        
        return !this->cell_domains[p].empty();
    };

    const auto row = this->board.getLine(pos.row, {0, 1});
    for (const auto& p : *row) {
        if (p.col != pos.col && !refine_domain(p)) {
            return false;
        }
    }

    const auto col = this->board.getLine(pos.col, {1, 0});
    for (const auto& p : *col) {
        if (p.row != pos.row && !refine_domain(p)) {
            return false;
        }
    }

    const auto box = this->board.getBox(this->board.getCellBox(pos));
    for (const auto& p : *box) {
        if (p != pos && !refine_domain(p)) {
            return false;
        }
    }

    const auto cage = this->board.getCellCage(pos);
    if (cage != nullptr) {
        for (const auto& p : cage->cells) {
            if (p != pos && !refine_domain(p)) {
                return false;
            }
        }
    }

    return true;
}
