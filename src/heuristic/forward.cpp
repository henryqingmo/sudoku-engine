#include <span>

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
        const auto iteration_deltas = std::span(
            this->deltas.top().first.get(), this->deltas.top().second
        );

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
    
    this->deltas.emplace(std::make_unique<DomainDelta[]>(delta_capacity), 1);

    const auto& iteration_deltas = this->deltas.top().first;
    std::bitset<BOARD_SIZE * BOARD_SIZE> is_refined;

    iteration_deltas[0] = {pos, this->cell_domains[pos]};
    this->cell_domains[pos] = {new_value};
    is_refined[pos.toOffset()] = true;

    std::size_t& delta_count = this->deltas.top().second;
    const auto refine_domain = [&](const BoardPosition& p) -> bool {
        // Already-refined domains will be ignored...
        if (is_refined[p.toOffset()])
            return true;

        auto& domain = this->cell_domains[p];

        if (!domain.has(new_value))
            return !domain.empty();

        if (delta_count >= delta_capacity) {
            throw std::runtime_error("Too many deltas for iteration");
        }

        iteration_deltas[delta_count++] = {p, domain};

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
