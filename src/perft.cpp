#include <cassert>
#include <chrono>
#include <fstream>
#include <iostream>
#include <string>

#include "game.h"
#include "movegen.h"
#include "str_util.h"
#include "types.h"

namespace choco {
template<bool print, bool debug>
uint64_t perft(Game& game, int depth) {
    static constexpr long double MS_IN_SECONDS = 1000;

    uint64_t nodesSearched = 0;

    std::chrono::time_point<std::chrono::steady_clock> start;
    if constexpr (debug) start = std::chrono::steady_clock::now();

    MoveList moveList;
    generateMoves<MoveType::LEGAL>(game, moveList);

    if (depth == 1) {
        nodesSearched = moveList.size();

        if constexpr (print) {
            for (const Move& move : moveList) {
                std::cout << move << " 1" << std::endl;
            }
        }
    } else {
        for (const Move& move : moveList) {
            UnmakeMove unmakeMove = game.makeMove(move);

            uint64_t nodesThisTime = perft<false, false>(game, depth - 1);

            nodesSearched += nodesThisTime;
            game.unmakeMove(unmakeMove);

            if constexpr (print) {
                std::cout << move << " " << std::to_string(nodesThisTime)
                          << std::endl;
            }
        }
    }

    if constexpr (print) {
        std::cout << "\n" << std::to_string(nodesSearched) << std::endl;
    }

    if constexpr (debug) {
        std::chrono::time_point<std::chrono::steady_clock> stop =
            std::chrono::steady_clock::now();
        auto durationMs =
            std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);

        std::cout << "\n--PERFT STATISTICS--\n";

        std::cout << "  Time elapsed: " << durationMs.count() << "ms\n";
        std::cout << "  Average NPS: "
                  << nodesSearched / ((durationMs.count() + 1) / MS_IN_SECONDS)
                  << "\n";
    }

    return nodesSearched;
}

template uint64_t perft<true, false>(Game& board, int depth);

template uint64_t perft<true, true>(Game& board, int depth);

template uint64_t perft<false, false>(Game& board, int depth);

struct ExpectedPerftTestResult {
    int depth;
    uint64_t nodes;
};

struct PerftTest {
    std::string fen;
    std::vector<ExpectedPerftTestResult> perftTestResults;
};

void runFullPerftTest(const std::string& fileName) {
    std::ifstream file(fileName);

    if (file.is_open()) {
        std::vector<PerftTest> tests;

        std::string line;
        int lines = 0;
        while (std::getline(file, line)) {
            lines++;

            std::vector<std::string> split = util::split(line, " ;");

            if (split.size() < 2) {
                std::cout << "Malformed line " << lines << std::endl;
                return;
            }

            // fen
            const std::string& fen = split[0];

            std::vector<ExpectedPerftTestResult> perftTestResults;

            // depths
            for (auto it = split.begin() + 1; it != split.end(); it++) {
                std::vector<std::string> taskSplit = util::split(*it, " ");
                if (taskSplit.size() != 2) {
                    std::cout << "Malformed line " << lines << std::endl;
                    return;
                }

                int depth;
                uint64_t nodes;

                try {
                    depth = std::stoi(taskSplit[0].substr(1));
                    nodes = std::stoull(taskSplit[1]);
                } catch (const std::invalid_argument& _) {
                    std::cout << "Malformed line " << lines << std::endl;
                    return;
                }

                perftTestResults.push_back({depth, nodes});
            }

            tests.push_back({fen, perftTestResults});
        }
        file.close();

        int failed = 0;

        for (const PerftTest& test : tests) {
            std::cout << "test \"" << test.fen << "\":" << std::endl;

            for (const auto& [depth, nodes] : test.perftTestResults) {
                std::cout << "  depth " << depth << ": " << nodes << "..."
                          << std::flush;

                Game game = Game(test.fen);

                uint64_t perftRes = perft<false, false>(game, depth);

                std::cout << perftRes;

                if (perftRes == nodes) {
                    std::cout << " GOOD" << std::endl;
                } else {
                    std::cout << " FAIL" << std::endl;
                    failed++;
                    break;
                }
            }
        }

        // summary
        std::cout << failed << " failed" << std::endl;
    } else {
        std::cout << "Could not open file" << std::endl;
    }
}
} // namespace choco
