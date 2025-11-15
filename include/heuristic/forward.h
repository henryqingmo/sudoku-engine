#pragma once

#include <list>
#include <memory>
#include <stack>
#include <tuple>
#include <unordered_map>
#include <vector>

#include "../utils.h"
#include "backtrack.h"

SUDOKU_NAMESPACE {
    class ForwardHeuristic final : public BacktrackHeuristic {
    private:
        using DomainDelta = std::pair<BoardPosition, BoardCellDomain>;
        using DomainDeltas = BoardState<BoardCellDomain>;

        // using DomainDeltas = utils::ArrayVector<DomainDelta>;

        struct RefinedDomains {
            DomainDeltas new_domains;
            BoardOffset values_pruned;
        };

        BoardState<BoardCellDomain> cell_domains;
        bool mrv, lcv;

    public:
        ForwardHeuristic(Board& board, bool mrv, bool lcv);
        ~ForwardHeuristic() = default;

        bool solve() override;

    protected:
        bool expand(const BoardPosition& pos) override;

    private:
        bool forwardCheck(
            RefinedDomains& result,
            const BoardPosition& pos,
            bool recursive
        ) const;

        BoardCellDomain getValidCageValues(const BoardCage& cage) const;
        BoardPosition findMrvCell() const;

        void applyDeltas(DomainDeltas&& deltas) {
            this->cell_domains = std::move(deltas);
        }

        DomainDeltas applyDeltasWithBackup(DomainDeltas&& deltas) {
            DomainDeltas originals = this->cell_domains.clone();
            this->cell_domains = std::move(deltas);
            return originals;
        }
    };
}
