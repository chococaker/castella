#pragma once

#include <cstdint>

#include "types.h"

namespace choco {
class Game;
struct CastlingRights;
} // namespace choco

namespace choco::zobrist {
class ZobristKey {
  public:
    ZobristKey() = default;
    explicit ZobristKey(const Game& game);
    explicit ZobristKey(uint64_t key);

    void flipSideToMove();

    void addPiece(Piece piece, Color color, Position pos);

    void removePiece(Piece piece, Color color, Position pos);

    void movePiece(Piece piece, Color color, Position from, Position to);

    void updateCastling(CastlingRights castlingRights);

    void updateEnPassant(uint8_t epFile);

    uint64_t getVal() const;

    bool operator==(const ZobristKey& other) const = default;

    bool operator!=(const ZobristKey& other) const = default;

  private:
    uint64_t val = 0;
};
} // namespace choco::zobrist
