#pragma once

#include <algorithm>
#include <cmath>

#include "eval/eval.h"
#include "types.h"
#include "zobrist.h"

namespace choco {
struct LittleMove {
    LittleMove();
    explicit LittleMove(const Move& move);

    Move toMove(const Game& game) const;
    bool isValid() const;

    uint16_t data;
};

struct TTEntry {
    zobrist::ZobristKey zkey;
    int32_t eval = 0;
    int16_t depth = -1;
    LittleMove bestMove = LittleMove();

    enum class Flag : uint8_t { EXACT, ALPHA, BETA };

    Flag flag = Flag::EXACT;

    bool exists() const {
        return depth != -1;
    }
};

class TT {
  public:
    TT();

    TT(const TT& other);

    TTEntry& probe(zobrist::ZobristKey zkey);

    void store(zobrist::ZobristKey zkey, int32_t eval, int16_t depth,
               const Move& bestMove, TTEntry::Flag flag);

    bool lookup(zobrist::ZobristKey zkey, TTEntry& out, int32_t requiredDepth);

    void clear();

    size_t size() const;

    TT& operator=(const TT& other);

    ~TT();

  private:
    constexpr static size_t TT_SHIFTS = 24;
    constexpr static size_t TT_SIZE = 1UL << TT_SHIFTS;
    constexpr static uint64_t TT_MASK = TT_SIZE - 1;

    size_t storedElements;
    TTEntry* table;
};

inline LittleMove::LittleMove() : data(0) {}

inline LittleMove::LittleMove(const Move& move) : data(0) {
    if (move.isValid()) {
        data |= move.from.getValue() << 6;
        data |= move.to.getValue();

        //  promotion
        if (move.promotionType != Piece::INVALID) {
            data |= (static_cast<uint16_t>(move.promotionType)) << 12;
        }
    }
}

inline Move LittleMove::toMove(const Game& game) const {
    Position from = Position((data & 0xFC0) >> 6);
    Position to = Position(data & 0x3F);

    if (uint8_t promotionData = data >> 12) {
        return {getPieceOf(game.getPieceColorPairAtPos(from)), from, to,
                static_cast<Piece>(promotionData)};
    } else {
        return {getPieceOf(game.getPieceColorPairAtPos(from)), from, to};
    }
}

inline bool LittleMove::isValid() const {
    return data != 0;
}

inline TT::TT() : storedElements(0) {
    table = new TTEntry[TT_SIZE];
    clear();
}

inline TT::TT(const TT& other) : storedElements(other.storedElements) {
    table = new TTEntry[TT_SIZE];
    std::copy_n(other.table, TT_SIZE, table);
}

// ReSharper disable once CppMemberFunctionMayBeConst
inline TTEntry& TT::probe(zobrist::ZobristKey zkey) {
    return table[zkey.getVal() & TT_MASK];
}

inline void TT::store(zobrist::ZobristKey zkey, int32_t eval, int16_t depth,
                      const Move& bestMove, TTEntry::Flag flag) {
    if (TTEntry& e = probe(zkey); depth > e.depth) {
        // new element (depth == -1), increased stored element count
        if (!e.exists()) storedElements++;

        e.zkey = zkey;
        e.eval = eval;
        e.depth = depth;
        e.bestMove = LittleMove(bestMove);
        e.flag = flag;
    }
}

inline bool TT::lookup(zobrist::ZobristKey zkey, TTEntry& out,
                       int32_t requiredDepth) {
    if (TTEntry& e = probe(zkey); e.zkey == zkey && e.depth >= requiredDepth) {
        out = e;
        return true;
    }
    return false;
}

inline void TT::clear() {
    std::fill_n(table, TT_SIZE, TTEntry());
    storedElements = 0;
}

inline size_t TT::size() const {
    return storedElements;
}

inline TT& TT::operator=(const TT& other) {
    if (this == &other) return *this;

    std::copy_n(other.table, TT_SIZE, table);
    storedElements = other.storedElements;
    return *this;
}

inline TT::~TT() {
    delete[] table;
}
} // namespace choco
