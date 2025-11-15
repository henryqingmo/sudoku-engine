#pragma once

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
        using DomainDeltas = utils::ArrayVector<DomainDelta>;

        struct RefinedDomains {
            DomainDeltas new_domains;
            BoardOffset values_pruned;
            bool is_legal;
        };

        // using DomainDeltas = std::unordered_map<
        //     BoardPosition,
        //     BoardCellDomain,
        //     BoardPosition::Hasher>;

        BoardState<BoardCellDomain> cell_domains;
        bool mrv, lcv;

    public:
        ForwardHeuristic(Board& board, bool mrv, bool lcv);
        ~ForwardHeuristic() = default;

        bool solve() override;

    protected:
        bool expand(const BoardPosition& pos) override;

    private:
        RefinedDomains forwardCheck(const BoardPosition& pos, bool recursive) const;

        BoardCellDomain getValidCageValues(const BoardCage& cage) const;
        BoardPosition findMrvCell() const;

        void applyDeltas(DomainDeltas&& deltas) {
            for (auto& [p, domain] : deltas.data()) {
                this->cell_domains[p] = std::move(domain);
            }
        }

        DomainDeltas applyDeltasWithBackup(DomainDeltas&& deltas) {
            DomainDeltas originals(deltas.size());
            for (auto& [p, new_domain] : deltas.data()) {
                auto& domain = this->cell_domains[p];
                originals.append(std::make_pair(p, std::move(domain)));
                domain = std::move(new_domain);
            }
            return originals;
        }
    };
}
