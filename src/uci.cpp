#include "uci.h"

#include <cmath>
#include <iostream>
#include <string>
#include <vector>

#include "eval/eval.h"
#include "game.h"
#include "perft.h"
#include "str_util.h"

namespace choco {
UciOptions::UciOptions(std::initializer_list<Option> optionList) {
    for (const Option& option : optionList) {
        options[option.name] = option;
    }
}

UciInstance::UciInstance() : options({}) {
    game = Game(STARTING_FEN);
    search.setGame(game, true);
    search.setThreadCount(4);

    options = UciOptions({{"Threads", "spin", "4", "1", "256"}});
}

void UciInstance::processLine(std::string line) {
    util::rtrim(line);

    std::vector<std::string> tokens = util::split(line, " ");

    if (tokens.empty()) {
        return;
    }

    if (tokens[0] == "uci") {
        uci();
    } else if (tokens[0] == "setoption") {
        setOption(line);
    } else if (tokens[0] == "position") {
        position(line);
    } else if (tokens[0] == "playmove") {
        playMove(line);
    } else if (tokens[0] == "go") {
        go(line);
    } else if (tokens[0] == "eval") {
        eval();
    } else if (tokens[0] == "stop") {
        stop();
    } else if (tokens[0] == "ponderhit") {
    } else if (tokens[0] == "ucinewgame") {
        uciNewGame();
    } else if (tokens[0] == "isready") {
        isReady();
    } else if (tokens[0] == "quit") {
        quit();
    } else if (tokens[0] == "bench") {
        bench(line);
    } else if (tokens[0] == "benchstat") {
        benchstat(line);
    } else if (tokens[0] == "benchtest") {
        benchtest(line);
    } else if (tokens[0] == "printgame") {
        printGame();
    }
}

void UciInstance::uci() {
    std::cout << "id name Silvertail" << std::endl;
    std::cout << "id author chococaker" << std::endl;
    for (const auto& [name, option] : options) {
        std::cout << "option name " << name << " type " << option.type
                  << " default " << option.defaultVal;
        if (!option.min.empty()) {
            std::cout << " min " << option.min << " max " << option.max;
        }
        std::cout << std::endl;
    }
    std::cout << "uciok" << std::endl;
}

void UciInstance::position(const std::string& line) {
    const int fenRange = util::findRange(line, "fen", "moves");

    std::string fen = line.find("fen") != std::string::npos
                          ? line.substr(line.find("fen") + 4, fenRange)
                          : STARTING_FEN;

    const std::string moves = line.find("moves") != std::string::npos
                                  ? line.substr(line.find("moves") + 6)
                                  : "";

    game = Game(fen);

    for (const std::vector<std::string> moveVec = util::split(moves, " ");
         const std::string& move : moveVec) {
        game.makeMove(uciToMove(game, move));
    }

    search.setGame(game, true);
}

void UciInstance::playMove(const std::string& line) {
    game.makeMove(uciToMove(game, util::split(line, " ")[1]));
    search.setGame(game, true);
}

void UciInstance::uciNewGame() {
    search.setGame(game, true);
}

void UciInstance::go(const std::string& line) {
    using namespace std::chrono_literals;

    std::vector<std::string> lineSplit = util::split(line, " ");

    int32_t maxTime =
        util::findElement<int32_t>(lineSplit, "movetime").value_or(0LL);
    int16_t maxDepth = static_cast<int16_t>(
        util::findElement<int32_t>(lineSplit, "depth").value_or(200L));

    Color engineSide = game.getGameState().sideToPlay;
    std::string timeParam = engineSide == Color::WHITE ? "wtime" : "btime";
    std::string incParam = engineSide == Color::WHITE ? "winc" : "binc";

    int32_t clockTime =
        util::findElement<int32_t>(lineSplit, timeParam).value_or(0LL);
    int32_t clockInc =
        util::findElement<int32_t>(lineSplit, incParam).value_or(0LL);

    SearchBounds searchBounds = {maxTime * 1ms, maxDepth, clockTime * 1ms,
                                 clockInc * 1ms};

    search.startSearch(searchBounds);
}

void UciInstance::eval() {
    using namespace std::chrono_literals;

    std::cout << "res " << eval::staticEval(game) << std::endl;
}

void UciInstance::quit() {
    search.endSearch();
}

void UciInstance::stop() {
    search.endSearch();
}

void UciInstance::isReady() { // NOLINT(*-convert-member-functions-to-static)
    std::cout << "readyok" << std::endl;
}

void UciInstance::setOption(const std::string& line) {
    std::vector<std::string> split = util::split(line, " ");

    std::string name =
        util::findElement<std::string>(split, "name").value_or("");
    if (name.empty()) return;

    std::string value =
        util::findElement<std::string>(split, "value").value_or("");

    if (name == "Threads") {
        int threadCount;

        try {
            threadCount = std::stoi(value);
        } catch (const std::invalid_argument& _) {
            return;
        }

        search.setThreadCount(threadCount);
    }
}

void UciInstance::bench(const std::string& line) {
    std::vector<std::string> tokens = util::split(line, " ");
    if (tokens.size() < 2) return;
    int depth = 0;
    try {
        depth = std::stoi(tokens[1]);
    } catch (...) {
        return;
    }

    perft<true, false>(game, depth);
}

void UciInstance::benchstat(const std::string& line) {
    std::vector<std::string> tokens = util::split(line, " ");
    if (tokens.size() < 2) return;
    int depth = 0;
    try {
        depth = std::stoi(tokens[1]);
    } catch (...) {
        return;
    }

    perft<true, true>(game, depth);
}

void UciInstance::benchtest(const std::string& line) {
    std::vector<std::string> tokens = util::split(line, " ");

    if (tokens.size() < 2) return;

    const std::string& fileName = tokens[1];

    runFullPerftTest(fileName);
}

void UciInstance::printGame() {
    std::cout << game << std::endl;
}

void uciInfo(int16_t depth, int32_t scoreCp, uint32_t nodes, uint16_t timeMs,
             const MoveList& pv) {
    std::cout << "info depth " << depth;

    if (std::abs(scoreCp) > eval::MATE_EVAL_THRESHOLD) {
        int32_t mateIn = eval::MATE_EVAL - std::abs(scoreCp) + 1;
        if (scoreCp < 0) mateIn *= -1;
        mateIn /= 2; // moves, not plies
        std::cout << " score mate " << mateIn;
    } else {
        std::cout << " score cp " << scoreCp;
    }
    std::cout << " time " << timeMs << " nodes " << nodes << " nps "
              << nodes * 1000 / (timeMs + 1) << " pv " << pv << std::endl;
}

void uciBestMove(const Move& move) {
    std::cout << "bestmove " << move << std::endl;
}

Move uciToMove(const Game& game, const std::string& str) {
    std::string fromStr = str.substr(0, 2);
    std::string toStr = str.substr(2, 2);

    Position from = Position((fromStr[0] - 'a') + (fromStr[1] - '1') * 8);
    Position to = Position((toStr[0] - 'a') + (toStr[1] - '1') * 8);

    Piece piece = getPieceOf(game.getPieceColorPairAtPos(from));

    Move move = {piece, from, to, Piece::INVALID};

    if (str.size() == 4) return move;

    switch (str[4]) {
    case 'q':
        move.promotionType = Piece::QUEEN;
        break;
    case 'n':
        move.promotionType = Piece::KNIGHT;
        break;
    case 'b':
        move.promotionType = Piece::BISHOP;
        break;
    case 'r':
        move.promotionType = Piece::ROOK;
        break;
    default:
        assert(0);
    }

    return move;
}
} // namespace choco
