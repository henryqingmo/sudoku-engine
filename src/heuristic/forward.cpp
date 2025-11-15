#include <algorithm>
#include <cassert>

#include "heuristic/forward.h"

using sudoku_engine::BoardCellDomain;
using sudoku_engine::BoardPosition;
using sudoku_engine::ForwardHeuristic;

ForwardHeuristic::ForwardHeuristic(Board& board, bool mrv, bool lcv)
    : BacktrackHeuristic(board), cell_domains(BOARD_SIZE, ~BoardCellDomain()),
      mrv(mrv), lcv(lcv) {
    BoardPosition pos = {0, 0};
    for (pos.row = 0; pos.row < BOARD_SIZE; pos.row++) {
        for (pos.col = 0; pos.col < BOARD_SIZE; pos.col++) {
            if (auto val = this->board.getValues()[pos]; val != CELL_EMPTY) {
                this->cell_domains[pos] = {val};
            }
        }
    }
}

bool ForwardHeuristic::forwardCheck(
    RefinedDomains& result,
    const BoardPosition& pos,
    const bool recursive
) const {
    constexpr std::size_t ITERATION_DELTA_MIN = 9 + 8 + 4;

    const auto cage = this->board.getCellCage(pos);

    std::size_t delta_capacity = ITERATION_DELTA_MIN;
    if (cage != nullptr) {
        delta_capacity += cage->cells.size() - 1;
    }

    DomainDeltas& iteration_deltas = result.new_domains;

    const auto refine_domain_raw = [&](const BoardPosition& p,
                                       const BoardCellDomain& new_domain,
                                       const BoardOffset delta_size) -> bool {
        iteration_deltas[p] = new_domain;
        result.values_pruned += delta_size;

        if (new_domain.empty()) {
            return false;
        }

        if (!recursive || p == pos) {
            return true;
        }

        if (!this->forwardCheck(result, p, recursive)) {
            return false;
        }

        return true;
    };

    const auto refine_domain = [&](const BoardPosition& p) -> bool {
        // Already-refined domains will be ignored...
        if (is_refined[p.toOffset()])
            return true;

        auto domain = this->cell_domains[p];

        if (!domain.has(new_value))
            return !domain.empty();

        domain.remove(new_value);
        return refine_domain_raw(p, domain, 1);
    };

    refine_domain_raw(pos, {new_value}, 0);

    if (cage != nullptr) {
        const auto cage_domain = this->getValidCageValues(*cage);
        for (const auto& p : cage->cells) {
            // Note that cage_domain only apply to empty cells!
            if (p == pos || this->board.getValues()[p] != CELL_EMPTY) {
                continue;
            }

            const auto& old_domain = this->cell_domains[p];
            auto domain = old_domain;

            if (domain.has(new_value)) {
                domain.remove(new_value);
            }

            domain = domain & cage_domain;

            BoardOffset delta_size =
                static_cast<BoardOffset>(old_domain.size() - domain.size());
            if (!refine_domain_raw(p, domain, delta_size)) {
                return false;
            }
        }
    }

    const auto row = this->board.getRow(pos.row);
    for (const auto& p : row) {
        if (!refine_domain(p)) {
            return false;
        }
    }

    const auto col = this->board.getCol(pos.col);
    for (const auto& p : col) {
        if (!refine_domain(p)) {
            return false;
        }
    }

    const auto box = this->board.getBox(this->board.getCellBox(pos));
    for (const auto& p : box) {
        if (!refine_domain(p)) {
            return false;
        }
    }

    return true;
}

BoardCellDomain ForwardHeuristic::getValidCageValues(
    const BoardCage& cage
) const {
    unsigned empty_cell_count = 0;
    long remaining_sum = cage.sum;

    for (const auto& cell_pos : cage.cells) {
        const auto value = this->board.getValues()[cell_pos];
        if (value == CELL_EMPTY) {
            empty_cell_count++;
        } else {
            remaining_sum -= value;
        }
    }

    if (empty_cell_count == 1) {
        // Only one cell left: it must be equal to remaining_sum
        if (remaining_sum >= CELL_MIN && remaining_sum <= CELL_MAX) {
            return {static_cast<BoardCell>(remaining_sum)};
        }
        return BoardCellDomain();
    }

    BoardCellDomain valid_values;

    const long min_remaining = (empty_cell_count - 1) * CELL_MIN;
    const long max_remaining = (empty_cell_count - 1) * CELL_MAX;

    // Check if each value could be part of a valid solution
    for (BoardCell value = CELL_MIN; value <= CELL_MAX; value++) {
        const long new_remaining = remaining_sum - value;

        if (new_remaining < 0) {
            continue;
        }

        if (new_remaining >= min_remaining && new_remaining <= max_remaining) {
            valid_values.add(value);
        }
    }

    return valid_values;
}

BoardPosition ForwardHeuristic::findMrvCell() const {
    const auto& cell_values = this->board.getValues();

    constexpr BoardCell DOMAIN_SIZE_INF = BoardCell(~0);
    static_assert(DOMAIN_SIZE_INF > CELL_MAX - CELL_MIN + 1);

    BoardPosition mrv_pos, p;
    BoardCell min_domain_size = DOMAIN_SIZE_INF;
    for (p.row = 0; p.row < BOARD_SIZE; p.row++) {
        for (p.col = 0; p.col < BOARD_SIZE; p.col++) {
            if (cell_values[p] != CELL_EMPTY) {
                continue;
            }
            const auto domain_size = this->cell_domains[p].size();
            if (domain_size < min_domain_size) {
                min_domain_size = domain_size;
                mrv_pos = p;
            }
        }
    }

    assert(min_domain_size > 0);
    if (min_domain_size == DOMAIN_SIZE_INF) {
        // No empty cells left...
        return {BOARD_SIZE, BOARD_SIZE};
    }

    return mrv_pos;
}

bool ForwardHeuristic::solve() {
    if (!this->mrv) {
        return this->expand({0, 0});
    }

    BoardPosition next_pos = this->findMrvCell();

    if (next_pos.row >= BOARD_SIZE) {
        // Board is already full
        return true;
    }

    return this->expand(next_pos);
}

bool ForwardHeuristic::expand(const BoardPosition& pos) {
    BoardPosition next_pos;

    if (this->mrv) {
        next_pos = this->findMrvCell();

        if (next_pos.row >= BOARD_SIZE) {
            // Board is full, we're done
            return true;
        }
    } else {
        if (pos.row == BOARD_SIZE) {
            // We've filled all rows, we're done
            return true;
        }

        next_pos = this->incrementPos(pos);
    }

    this->step_count++;

    auto& cell_value = this->board.getValues()[pos];

    // Skip cells that are already filled
    if (cell_value != CELL_EMPTY) {
        return this->expand(next_pos);
    }

    using ChildRefinement = std::pair<BoardCell, RefinedDomains>;
    utils::ArrayVector<ChildRefinement> child_refinements(BOARD_SIZE);

    // Try placing numbers 1-9
    for (BoardCell num = CELL_MIN; num <= CELL_MAX; num++) {
        cell_value = num;

        if (!this->cell_domains[pos].has(num) || this->board.isInvalid(pos)) {
            cell_value = CELL_EMPTY;
            continue;
        }

        auto refinement = this->forwardCheck(pos, false);
        if (!refinement.is_legal) {
            continue;
        }

        ChildRefinement&& child_refinement =
            std::make_pair(std::move(num), std::move(refinement));
        child_refinements.append(std::move(child_refinement));
    }

    cell_value = CELL_EMPTY;

    const auto refinement_span = child_refinements.data();

    if (this->lcv) {
        std::sort(
            refinement_span.begin(),
            refinement_span.end(),
            [](const ChildRefinement& a, const ChildRefinement& b) -> bool {
                return a.second.values_pruned < b.second.values_pruned;
            }
        );
    }

    for (auto& [num, refinement] : refinement_span) {
        cell_value = num;
        DomainDeltas unrefined =
            this->applyDeltasWithBackup(std::move(refinement.new_domains));

        // Recursively try to fill the rest if legal
        if (this->expand(next_pos)) {
            return true;
        }

        // Backtrack if this placement doesn't lead to a solution
        cell_value = CELL_EMPTY;
        this->applyDeltas(std::move(unrefined));
    }

    return false;
}
