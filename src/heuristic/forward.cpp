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

    this->deltas.push({{pos, this->cell_domains[pos]}});
    this->cell_domains[pos] = {new_value};

    DomainDeltas& iteration_deltas = this->deltas.top();

    const auto refine_domain = [&](const BoardPosition& p) -> bool {
        auto& domain = this->cell_domains[p];
        // Already-refined domains will be ignored...
        iteration_deltas.insert({p, domain});
        domain.remove(new_value);
        return !domain.empty();
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
