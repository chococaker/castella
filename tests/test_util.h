#pragma once

#include "game.h"
#include "uci.h"

inline choco::Game getTestGameA() {
    choco::Game game = choco::Game(choco::STARTING_FEN);

    return game;
}

inline choco::Game getTestGameB() {
    choco::Game game = choco::Game(choco::STARTING_FEN);

    choco::Move move = choco::uciToMove(game, "g1f3");
    game.makeMove(move);

    return game;
}

inline choco::Game getTestGameC() {
    choco::Game game = choco::Game("8/8/5k2/3n4/4P3/8/4K3/8 w - - 0 1");

    return game;
}

inline choco::Game getTestGameD() {
    choco::Game game = choco::Game("8/8/5k2/3n4/8/4N3/4K3/8 w - - 0 1");

    return game;
}

inline choco::Game getTestGameE() {
    choco::Game game = choco::Game("8/8/5k2/8/8/4p3/4P3/3K4 w - - 0 1");

    return game;
}

inline choco::Game getTestGameF() {
    choco::Game game = choco::Game("8/8/6k1/8/5p2/1PP2P2/8/2K5 w - - 0 1");

    return game;
}

inline choco::Game getTestGameG() {
    choco::Game game = choco::Game("8/8/2p3k1/8/5p2/1PP2P2/8/2K5 w - - 0 1");

    return game;
}

inline const choco::History& getEmptyHistory() {
    static constexpr choco::History EMPTY_HISTORY = choco::History();

    return EMPTY_HISTORY;
}
