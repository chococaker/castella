#pragma once

#include "game.h"

namespace choco {
template<bool print, bool debug>
uint64_t perft(Game& board, int depth);

void runFullPerftTest(const std::string& fileName);
} // namespace choco
