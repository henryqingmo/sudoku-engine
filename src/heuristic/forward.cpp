#include <span>

#include "heuristic/forward.h"

using sudoku_engine::BoardCellDomain;
using sudoku_engine::ForwardHeuristic;

ForwardHeuristic::ForwardHeuristic(Board& board)
    : BacktrackHeuristic(board), cell_domains(BOARD_SIZE, ~BoardCellDomain()) {
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
    return ~this->cell_domains[pos];
}

bool ForwardHeuristic::onUpdate(const BoardPosition& pos) {
    const BoardCell new_value = this->board.getValues()[pos];

    // Reset domains when backtracking...
    if (new_value == CELL_EMPTY) {
        const auto iteration_deltas = this->deltas.top().data();

        for (auto& [p, domain] : iteration_deltas) {
            this->cell_domains[p] = std::move(domain);
        }

        this->deltas.pop();
        return true;
    }

    constexpr std::size_t ITERATION_DELTA_MIN = 9 + 8 + 4;

    const auto cage = this->board.getCellCage(pos);

    std::size_t delta_capacity = ITERATION_DELTA_MIN;
    if (cage != nullptr) {
        delta_capacity += cage->cells.size() - 1;
    }

    this->deltas.emplace(delta_capacity);

    auto& iteration_deltas = this->deltas.top();
    std::bitset<BOARD_SIZE * BOARD_SIZE> is_refined;

    iteration_deltas.append({pos, this->cell_domains[pos]});
    this->cell_domains[pos] = {new_value};
    is_refined[pos.toOffset()] = true;

    const auto save_for_refine = [&](const BoardPosition& p,
                                    const BoardCellDomain& old_domain) -> void {
        iteration_deltas.append({p, old_domain});
        is_refined[p.toOffset()] = true;
    };

    const auto refine_domain = [&](const BoardPosition& p) -> bool {
        // Already-refined domains will be ignored...
        if (is_refined[p.toOffset()])
            return true;

        auto& domain = this->cell_domains[p];

        if (!domain.has(new_value))
            return !domain.empty();

        save_for_refine(p, domain);
        domain.remove(new_value);

        return !domain.empty();
    };

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

    if (cage != nullptr) {
        const auto cage_domain = this->getValidCageValues(*cage);
        for (const auto& p : cage->cells) {
            // if (!refine_domain(p)) {
            //     return false;
            // }

            // Note that cage_domain only apply to empty cells!
            if (p == pos || this->board.getValues()[p] != CELL_EMPTY) {
                continue;
            }

            auto& domain = this->cell_domains[p];

            if (!is_refined[p.toOffset()]) {
                save_for_refine(p, domain);
                domain.remove(new_value);
            }

            domain = domain & cage_domain;

            if (domain.empty()) {
                return false;
            }
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
