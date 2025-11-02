#pragma once

#include <memory>
#include <stack>
#include <tuple>
#include <unordered_map>
#include <vector>

#include "backtrack.h"

SUDOKU_NAMESPACE {
    class ForwardHeuristic final : public BacktrackHeuristic {
    private:
        using DomainDelta = std::pair<BoardPosition, BoardCellDomain>;
        using DomainDeltas =
            std::pair<std::unique_ptr<DomainDelta[]>, std::size_t>;

        // using DomainDeltas = std::unordered_map<
        //     BoardPosition,
        //     BoardCellDomain,
        //     BoardPosition::Hasher>;

        BoardState<BoardCellDomain> cell_domains;
        std::stack<DomainDeltas> deltas;

    public:
        ForwardHeuristic(Board& board);
        ~ForwardHeuristic() = default;

    protected:
        BoardCellDomain getInvalidChoices(
            const BoardPosition& pos
        ) const override;
        bool onUpdate(const BoardPosition& pos) override;
    };
}
