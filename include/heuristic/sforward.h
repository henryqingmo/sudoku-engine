#pragma once

#include <memory>
#include <stack>
#include <tuple>
#include <unordered_map>
#include <vector>

#include "../utils.h"
#include "forward.h"
#include "sbacktrack.h"

SUDOKU_NAMESPACE {
    class SForwardHeuristic : public SBacktrackHeuristic,
                              public ForwardHeuristic {
    private:
        struct ForwardFrame {
            enum State {
                Init,
                GeneratedCandidates,
                TryingCandidate
            } state = Init;
            BoardPosition pos{};  // position this frame is deciding
            bool skip_cell = false;
            // Candidate list: pair<num, refinement>
            std::vector<std::pair<BoardCell, RefinedDomains>> candidates;
            size_t try_index = 0;  // index of next candidate to try
            // backup returned by applyDeltasWithBackup for the currently-tried candidate
            DomainDeltas current_unrefined;
            bool has_current_unrefined = false;
        };

        std::vector<ForwardFrame> forward_stack;
        bool forward_stepping_initialized = false;

    public:
        SForwardHeuristic(
            Board& board,
            std::size_t step_limit,
            bool mrv,
            bool lcv
        )
            : BacktrackHeuristic(board, step_limit),
              SBacktrackHeuristic(board, step_limit),
              ForwardHeuristic(board, step_limit, mrv, lcv) {};

        ~SForwardHeuristic() = default;

        StepResult step() override;

        BoardCellDomain getDomain(const BoardPosition& pos) const override {
            return this->cell_domains[pos];
        }

    private:
        void startSteppingForward();
    };
}
