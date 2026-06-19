#pragma once

#include "bitboard.h"
#include "types.h"

namespace choco::attacks {
void initAttacks();

Bitboard king(Position pos);
Bitboard queen(Position pos, Bitboard occupied);
Bitboard bishop(Position pos, Bitboard occupied);
Bitboard knight(Position pos);
Bitboard rook(Position pos, Bitboard occupied);
Bitboard pawn(Position pos, Color mover);

Bitboard pawnAttackersTo(Bitboard pawnBB, Color mover, Bitboard attackedBB);

template<Piece piece>
Bitboard attacks(Position pos, Bitboard occupied);

Bitboard attacks(Piece piece, Position pos, Bitboard occupied);

Bitboard inBetweenSquares(Position from, Position to);

/**
 * Returns whether three positions are along a line
 */
bool areAlongLine(Position p1, Position p2, Position p3);
} // namespace choco::attacks
