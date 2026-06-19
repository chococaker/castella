#include "move_order.h"
#include "game.h"
#include "movegen.h"
#include "test_util.h"

#include <doctest/doctest.h>

TEST_CASE("Move order functionality") {
    SUBCASE("Starting position falls through to valid move") {
        choco::Game game = getTestGameA();

        choco::MoveOrder moveOrder =
            choco::MoveOrder(game, choco::INVALID_MOVE, getEmptyHistory());

        choco::Move generatedMove = moveOrder.selectMove();
        CHECK(generatedMove.isValid());
        CHECK(choco::isPseudoLegal(generatedMove, game));
    }
}
