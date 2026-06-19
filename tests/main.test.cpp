#define DOCTEST_CONFIG_IMPLEMENT
#include <doctest.h>

#include "attacks.h"

int main(int argc, char* argv[]) {
    // Global setup
    choco::attacks::initAttacks();

    doctest::Context ctx;
    int res = ctx.run();

    return res;
}
