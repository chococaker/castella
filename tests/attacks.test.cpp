#include <doctest.h>

#include "attacks.h"

TEST_CASE("inBetweenSquares functionality") {
    SUBCASE("Two positions next to each other returns no in-between squares") {
        choco::Bitboard ibSqBB =
            choco::attacks::inBetweenSquares(choco::E1, choco::E2);
        CHECK_EQ(ibSqBB.count(), 0);
    }

    SUBCASE("Two positions vertically one space apart has the correct "
            "in-between square") {
        choco::Bitboard ibSqBB =
            choco::attacks::inBetweenSquares(choco::E1, choco::E3);
        CHECK_EQ(ibSqBB, choco::Bitboard::fromPos(choco::E2));
    }

    SUBCASE("Two positions horizontally one space apart has the correct "
            "in-between square") {
        choco::Bitboard ibSqBB =
            choco::attacks::inBetweenSquares(choco::E1, choco::G1);
        CHECK_EQ(ibSqBB, choco::Bitboard::fromPos(choco::F1));
    }

    SUBCASE("Two positions horizontally on opposite sides of the board have "
            "the correct in-between squares") {
        choco::Bitboard ibSqBB =
            choco::attacks::inBetweenSquares(choco::A1, choco::H1);
        choco::Bitboard correctBB = choco::BB_RANK_1 ^
                                    choco::Bitboard::fromPos(choco::A1) ^
                                    choco::Bitboard::fromPos(choco::H1);
        CHECK_EQ(ibSqBB, correctBB);
    }

    SUBCASE("A1 to H8 have the correct in-between squares") {
        choco::Bitboard ibSqBB =
            choco::attacks::inBetweenSquares(choco::A1, choco::H8);
        choco::Bitboard ibSqRevBB =
            choco::attacks::inBetweenSquares(choco::H8, choco::A1);
        choco::Bitboard correctBB = choco::Bitboard(0x40201008040200);
        CHECK_EQ(ibSqBB, correctBB);
        CHECK_EQ(ibSqRevBB, correctBB);
    }
    SUBCASE("A8 to H1 have the correct in-between squares") {
        choco::Bitboard ibSqBB =
            choco::attacks::inBetweenSquares(choco::A8, choco::H1);
        choco::Bitboard ibSqRevBB =
            choco::attacks::inBetweenSquares(choco::H1, choco::A8);
        choco::Bitboard correctBB = choco::Bitboard(0x2040810204000);
        CHECK_EQ(ibSqBB, correctBB);
        CHECK_EQ(ibSqRevBB, correctBB);
    }

    SUBCASE("G3 to E5 have the correct in-between square") {
        choco::Bitboard ibSqBB =
            choco::attacks::inBetweenSquares(choco::G3, choco::E5);
        choco::Bitboard ibSqRevBB =
            choco::attacks::inBetweenSquares(choco::E5, choco::G3);
        choco::Bitboard correctBB = choco::Bitboard::fromPos(choco::F4);
        CHECK_EQ(ibSqBB, correctBB);
        CHECK_EQ(ibSqRevBB, correctBB);
    }

    SUBCASE("D2 and F4 have correct in-between square") {
        choco::Bitboard ibSqBB =
            choco::attacks::inBetweenSquares(choco::D2, choco::F4);
        choco::Bitboard ibSqRevBB =
            choco::attacks::inBetweenSquares(choco::F4, choco::D2);
        choco::Bitboard correctBB = choco::Bitboard::fromPos(choco::E3);
        CHECK_EQ(ibSqBB, correctBB);
        CHECK_EQ(ibSqRevBB, correctBB);
    }

    SUBCASE("C2 and C8 have correct in-between squares") {
        choco::Bitboard ibSqBB =
            choco::attacks::inBetweenSquares(choco::C2, choco::C8);
        choco::Bitboard ibSqRevBB =
            choco::attacks::inBetweenSquares(choco::C8, choco::C2);
        choco::Bitboard correctBB = choco::Bitboard(0x4040404040000);
        CHECK_EQ(ibSqBB, correctBB);
        CHECK_EQ(ibSqRevBB, correctBB);
    }
}

TEST_CASE("areAlongLine functionality") {
    SUBCASE("A1, B1 and C1 are along a line") {
        CHECK(choco::attacks::areAlongLine(choco::A1, choco::B1, choco::C1));
    }

    SUBCASE("A1, C3 and F6 are along a line") {
        CHECK(choco::attacks::areAlongLine(choco::A1, choco::C3, choco::F6));
    }

    SUBCASE("E8, B8 and A8 are along a line") {
        CHECK(choco::attacks::areAlongLine(choco::E8, choco::B8, choco::A8));
    }
}
