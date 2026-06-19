#include "zobrist.h"

#include "game.h"

namespace choco::zobrist {
class XorShift64Star {
  public:
    constexpr explicit XorShift64Star(uint64_t seed)
        : state(seed ? seed : 0xDEADBEEFCAFEBABEULL) {}

    constexpr uint64_t next() {
        uint64_t x = state;
        x ^= x >> 12;
        x ^= x << 25;
        x ^= x >> 27;
        state = x;
        return x * 0x2545F4914F6CDD1DULL;
    }

  private:
    uint64_t state;
};

struct ZobristKeys {
    uint64_t blackToMove;
    uint64_t pieces[2][6][64];
    uint64_t castlingRights[16];
    uint64_t enPassantFile[8];
};

constexpr ZobristKeys genZKeys() {
    // Originally used my own random number, but TT clashes became much less
    // common
    constexpr uint64_t SEED = 49109794719ULL;

    ZobristKeys zkeys = {};

    XorShift64Star rand {SEED};

    for (auto& color : zkeys.pieces)
        for (auto& piece : color)
            for (uint64_t& square : piece)
                square = rand.next();

    for (uint64_t& castlingRight : zkeys.castlingRights)
        castlingRight = rand.next();

    for (uint64_t& i : zkeys.enPassantFile)
        i = rand.next();

    zkeys.blackToMove = rand.next();

    return zkeys;
}

constexpr ZobristKeys ZKEYS = genZKeys();

ZobristKey::ZobristKey(const Game& game) {
    for (int c = 0; c < 2; c++) {
        for (int p = 0; p < 6; p++) {
            Piece piece = static_cast<Piece>(p);
            Color color = static_cast<Color>(c);

            Bitboard bb = game.getBB(color, piece);
            while (bb.hasAny()) {
                Position pos = bb.poplsb();
                addPiece(piece, color, pos);
            }
        }
    }

    if (game.getGameState().sideToPlay == Color::BLACK) {
        flipSideToMove();
    }

    updateCastling(game.getGameState().castlingRights);

    if (game.getGameState().enpassantPos != INVALID_POSITION) {
        updateEnPassant(game.getGameState().enpassantPos.file());
    }
}

ZobristKey::ZobristKey(uint64_t key) : val(key) {}

void ZobristKey::flipSideToMove() {
    val ^= ZKEYS.blackToMove;
}

void ZobristKey::addPiece(Piece piece, Color color, Position pos) {
    val ^= ZKEYS.pieces[static_cast<uint8_t>(color)]
                       [static_cast<uint8_t>(piece)][pos.getValue()];
}

void ZobristKey::removePiece(Piece piece, Color color, Position pos) {
    val ^= ZKEYS.pieces[static_cast<uint8_t>(color)]
                       [static_cast<uint8_t>(piece)][pos.getValue()];
}

void ZobristKey::movePiece(Piece piece, Color color, Position from,
                           Position to) {
    val ^= ZKEYS.pieces[static_cast<int>(color)][static_cast<int>(piece)]
                       [from.getValue()] ^
           ZKEYS.pieces[static_cast<int>(color)][static_cast<int>(piece)]
                       [to.getValue()];
}

void ZobristKey::updateCastling(CastlingRights castlingRights) {
    val ^= ZKEYS.castlingRights[castlingRights.getData()];
}

void ZobristKey::updateEnPassant(uint8_t epFile) {
    val ^= ZKEYS.enPassantFile[epFile];
}

uint64_t ZobristKey::getVal() const {
    return val;
}
} // namespace choco::zobrist
