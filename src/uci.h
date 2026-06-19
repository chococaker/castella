#pragma once

#include <string>
#include <unordered_map>

#include "game.h"
#include "search.h"
#include "types.h"

namespace choco {
static const std::string STARTING_FEN =
    "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

class UciOptions {
  public:
    struct Option {
        std::string name;
        std::string type;
        std::string defaultVal;
        std::string min;
        std::string max;
    };

    UciOptions(std::initializer_list<Option> optionList);

    std::unordered_map<std::string, Option>::iterator begin() {
        return options.begin();
    }

    std::unordered_map<std::string, Option>::iterator end() {
        return options.end();
    }

  private:
    std::unordered_map<std::string, Option> options;
};

class UciInstance {
  public:
    UciInstance();

    void processLine(std::string line);

  private:
    // commands
    void uci();

    void position(const std::string& line);

    void playMove(const std::string& line);

    void uciNewGame();

    void go(const std::string& line);

    void eval();

    void quit();

    void stop();

    void isReady();

    void setOption(const std::string& line);

    void bench(const std::string& line);

    void benchstat(const std::string& line);

    void benchtest(const std::string& line);

    void printGame();

    UciOptions options;
    Search search;
    Game game;
};

void uciInfo(int16_t depth, int32_t scoreCp, uint32_t nodes, uint16_t timeMs,
             const MoveList& pv);

void uciBestMove(const Move& move);

Move uciToMove(const Game& game, const std::string& str);
} // namespace choco
