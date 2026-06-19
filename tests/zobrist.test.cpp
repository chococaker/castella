#include "doctest.h"

#include "game.h"
#include "test_util.h"
#include "uci.h"

TEST_CASE("ZobristKey Functionality") {
    SUBCASE("Different games have different hashes") {
        choco::Game a = getTestGameA();

        choco::Game b = getTestGameB();

        CHECK_NE(a.getZKey(), b.getZKey());
    }

    SUBCASE("Unmaking a move results in the same hash") {
        choco::Game a = getTestGameA();
        choco::Game copy = a;

        choco::UnmakeMove um =
            a.makeMove(choco::Move(choco::Piece::PAWN, choco::E2, choco::E4));
        a.unmakeMove(um);

        CHECK_EQ(a.getZKey(), copy.getZKey());
    }
}
