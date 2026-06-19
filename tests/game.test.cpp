#include <doctest.h>

#include "game.h"
#include "uci.h"

#include <iostream>

TEST_CASE("isLegal functionality") {
    SUBCASE("Pinned piece can take checker") {
        choco::Game game = choco::Game("1q3RK1/8/2k5/8/8/8/8/8 w - - 0 1");
        choco::Move move = choco::uciToMove(game, "f8b8");

        CHECK(game.isLegal(move));
    }

    SUBCASE("Pinned piece cannot move out of ray") {
        choco::Game game = choco::Game("1q3RK1/8/2k5/8/8/8/8/8 w - - 0 1");
        choco::Move move = choco::uciToMove(game, "f8f7");

        CHECK_FALSE(game.isLegal(move));
    }

    SUBCASE("Kiwipete: pinned piece can take checker") {
        choco::Game game = choco::Game(
            "Qr2k2r/2ppqpb1/b3pnp1/3PN3/np2P3/2N4p/PPPBBPPP/R3K2R b KQk - 0 1");
        choco::Move move = choco::uciToMove(game, "b8a8");

        CHECK(game.isLegal(move));
    }
}

TEST_CASE("isRepetition functionality") {
    SUBCASE("White repetition") {
        choco::Game game = choco::Game("8/7k/3K2r1/8/8/8/8/8 w - - 0 1");
        choco::Move kingMoveUp = choco::Move(choco::Piece::KING, choco::D6, choco::D7);
        choco::Move rookMoveUp = choco::Move(choco::Piece::ROOK, choco::G6, choco::G7);
        choco::Move kingMoveDown = choco::Move(choco::Piece::KING, choco::D7, choco::D6);
        choco::Move rookMoveDown = choco::Move(choco::Piece::ROOK, choco::G7, choco::G6);

        game.makeMove(kingMoveUp);
        game.makeMove(rookMoveUp);
        game.makeMove(kingMoveDown);
        game.makeMove(rookMoveDown);

        game.makeMove(kingMoveUp);
        game.makeMove(rookMoveUp);
        game.makeMove(kingMoveDown);
        game.makeMove(rookMoveDown);

        CHECK(game.isRepetition(3));
    }
}
