#pragma once

#include "game.h"

namespace choco {
enum class MoveType {
    LEGAL,
    NOISY,
    QUIET,
};

template<MoveType moveType>
void generateMoves(const Game& game, MoveList& moves);

/**
 * A basic check for the pseudo-legality of a move against a game. Useful for
 * validating TT PV entries.
 */
bool isPseudoLegal(const Move& move, const Game& game);
} // namespace choco
