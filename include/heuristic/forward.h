#pragma once

#include <stack>
#include <tuple>
#include <vector>

#include "backtrack.h"

SUDOKU_NAMESPACE {
    class ForwardHeuristic final : public BacktrackHeuristic {
    private:
        using DomainDeltas =
            std::vector<std::pair<BoardPosition, BoardCellDomain>>;

        BoardState<BoardCellDomain> cell_domains;
        std::stack<DomainDeltas> deltas;

    public:
        ForwardHeuristic(Board& board);
        ~ForwardHeuristic() = default;

    protected:
        BoardCellDomain getInvalidChoices(const BoardPosition& pos) const override;
        bool onUpdate(const BoardPosition& pos) override;
    };
}
