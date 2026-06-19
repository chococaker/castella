#include "doctest.h"

#include "game.h"
#include "movegen.h"
#include "uci.h"

#include "test_util.h"

TEST_CASE("isPseudoLegal correctness") {
    SUBCASE("White legally pushing pawn yields valid move") {
        choco::Game game = getTestGameA();

        choco::Move move = choco::uciToMove(game, "e2e4");
        CHECK(choco::isPseudoLegal(move, game));
    }

    SUBCASE("Black legally pushing pawn yields valid move") {
        choco::Game game = getTestGameB();

        choco::Move move = choco::uciToMove(game, "e7e5");
        CHECK(choco::isPseudoLegal(move, game));
    }

    SUBCASE("White moving king into pawn yields invalid move") {
        choco::Game game = getTestGameA();

        choco::Move move = choco::uciToMove(game, "e1d2");
        CHECK(!choco::isPseudoLegal(move, game));
    }

    SUBCASE("White moving knight forward yields valid move") {
        choco::Game game = getTestGameA();

        choco::Move move = choco::uciToMove(game, "g1f3");
        CHECK(choco::isPseudoLegal(move, game));
    }

    SUBCASE("Black moving knight forward yields valid move") {
        choco::Game game = getTestGameB();

        choco::Move move = choco::uciToMove(game, "g8f6");
        CHECK(choco::isPseudoLegal(move, game));
    }

    SUBCASE("White capturing black with pawn yields valid move") {
        choco::Game game = getTestGameC();

        choco::Move move = choco::uciToMove(game, "e4d5");
        CHECK(choco::isPseudoLegal(move, game));
    }

    SUBCASE("White double pushing pawn yields valid move") {
        choco::Game game = getTestGameA();

        choco::Move move = choco::uciToMove(game, "e2e4");
        CHECK(choco::isPseudoLegal(move, game));
    }

    SUBCASE("White double pushing pawn into another yields invalid move") {
        choco::Game game = getTestGameE();

        choco::Move move = choco::uciToMove(game, "e2e4");
        CHECK_FALSE(choco::isPseudoLegal(move, game));
    }

    SUBCASE("White single pushing pawn into another yields invalid move") {
        choco::Game game = getTestGameE();

        choco::Move move = choco::uciToMove(game, "e2e3");
        CHECK_FALSE(choco::isPseudoLegal(move, game));
    }
}

TEST_CASE("MoveType correctness") {
    SUBCASE("Starting position NOISY moves are empty") {
        choco::Game game = getTestGameA();

        choco::MoveList moveList;
        choco::generateMoves<choco::MoveType::NOISY>(game, moveList);

        CHECK_EQ(moveList.size(), 0);
    }

    SUBCASE("Position with one capture has correct NOISY moves") {
        choco::Game game = choco::Game("k7/p4r2/8/8/8/8/5R2/5K2 w - - 0 1");

        choco::MoveList moveList;
        choco::generateMoves<choco::MoveType::NOISY>(game, moveList);

        REQUIRE_EQ(moveList.size(), 1);
        CHECK_EQ(moveList[0],
                 choco::Move(choco::Piece::ROOK, choco::F2, choco::F7));
    }
}
