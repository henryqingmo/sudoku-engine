#include "heuristic/forward.h"

using sudoku_engine::BoardCellDomain;
using sudoku_engine::ForwardHeuristic;

ForwardHeuristic::ForwardHeuristic(Board& board)
    : BacktrackHeuristic(board), cell_domains(BOARD_SIZE, -BoardCellDomain()) {
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
        for (auto& [p, domain] : this->deltas.top()) {
            this->cell_domains[p] = std::move(domain);
        }

        this->deltas.pop();
        return true;
    }

    this->deltas.push({});

    DomainDeltas& iteration_deltas = this->deltas.top();
    std::bitset<BOARD_SIZE * BOARD_SIZE> is_refined;

    constexpr std::size_t ITERATION_DELTA_MIN = 9 + 8 + 4;

    const auto cage = this->board.getCellCage(pos);
    if (cage != nullptr) {
        iteration_deltas.reserve(ITERATION_DELTA_MIN + cage->cells.size() - 1);
    } else {
        iteration_deltas.reserve(ITERATION_DELTA_MIN);
    }

    iteration_deltas.push_back({pos, this->cell_domains[pos]});
    this->cell_domains[pos] = {new_value};
    is_refined[pos.toOffset()] = true;

    const auto refine_domain = [&](const BoardPosition& p) -> bool {
        // Already-refined domains will be ignored...
        if (is_refined[p.toOffset()])
            return true;

        auto& domain = this->cell_domains[p];

        if (!domain.has(new_value))
            return !domain.empty();

        iteration_deltas.push_back({p, domain});

        domain.remove(new_value);
        is_refined[p.toOffset()] = true;

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
        for (const auto& p : cage->cells) {
            if (!refine_domain(p)) {
                return false;
            }
        }
    }

    return true;
}
