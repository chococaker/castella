#include "doctest.h"

#include "types.h"

TEST_CASE("Move functionality") {
    SUBCASE("INVALID_MOVE returns false for isValid()") {
        CHECK_FALSE(choco::INVALID_MOVE.isValid());
    }
}
