#pragma once

#include <cstdint>
#include <sstream>

#include "types.h"

namespace choco {
class Bitboard {
  public:
    constexpr Bitboard() = default;

    constexpr explicit Bitboard(uint64_t value);

    static constexpr Bitboard fromPos(Position pos);

    static constexpr Bitboard rankOf(uint8_t rank);

    static constexpr Bitboard fileOf(uint8_t file);

    constexpr Bitboard operator<<(uint32_t other) const;

    constexpr Bitboard operator>>(uint32_t other) const;

    constexpr Bitboard operator&(const Bitboard& other) const;

    constexpr Bitboard operator|(const Bitboard& other) const;

    constexpr Bitboard operator^(const Bitboard& other) const;

    constexpr Bitboard operator~() const;

    constexpr Bitboard& operator<<=(uint32_t other);

    constexpr Bitboard& operator>>=(uint32_t other);

    constexpr Bitboard& operator&=(const Bitboard& other);

    constexpr Bitboard& operator|=(const Bitboard& other);

    constexpr Bitboard& operator^=(const Bitboard& other);

    constexpr bool operator==(const Bitboard& other) const = default;

    constexpr bool operator!=(const Bitboard& other) const = default;

    constexpr bool hasPos(Position pos) const;

    constexpr bool hasAny() const;

    constexpr bool empty() const;

    constexpr uint8_t count() const;

    constexpr Position lsb() const;

    constexpr Bitboard lsbBB() const;

    constexpr Position poplsb();

    constexpr Bitboard poplsbBB();

    constexpr Bitboard up(Color perspective = Color::WHITE) const;

    constexpr Bitboard down(Color perspective = Color::WHITE) const;

    constexpr Bitboard left(Color perspective = Color::WHITE) const;

    constexpr Bitboard right(Color perspective = Color::WHITE) const;

    constexpr Bitboard flipPerspective() const;

    constexpr uint64_t getValue() const;

    friend std::ostream& operator<<(std::ostream& os, const Bitboard& bitboard);

  private:
    uint64_t value;
};

constexpr Bitboard::Bitboard(uint64_t value) : value(value) {}

static constexpr Bitboard BB_RANK_1 = Bitboard(0x00000000000000FFULL);
static constexpr Bitboard BB_RANK_2 = Bitboard(0x000000000000FF00ULL);
static constexpr Bitboard BB_RANK_3 = Bitboard(0x0000000000FF0000ULL);
static constexpr Bitboard BB_RANK_4 = Bitboard(0x00000000FF000000ULL);
static constexpr Bitboard BB_RANK_5 = Bitboard(0x000000FF00000000ULL);
static constexpr Bitboard BB_RANK_6 = Bitboard(0x0000FF0000000000ULL);
static constexpr Bitboard BB_RANK_7 = Bitboard(0x00FF000000000000ULL);
static constexpr Bitboard BB_RANK_8 = Bitboard(0xFF00000000000000ULL);

static constexpr Bitboard BB_FILE_A = Bitboard(0x0101010101010101ULL);
static constexpr Bitboard BB_FILE_B = Bitboard(0x0202020202020202ULL);
static constexpr Bitboard BB_FILE_C = Bitboard(0x0404040404040404ULL);
static constexpr Bitboard BB_FILE_D = Bitboard(0x0808080808080808ULL);
static constexpr Bitboard BB_FILE_E = Bitboard(0x1010101010101010ULL);
static constexpr Bitboard BB_FILE_F = Bitboard(0x2020202020202020ULL);
static constexpr Bitboard BB_FILE_G = Bitboard(0x4040404040404040ULL);
static constexpr Bitboard BB_FILE_H = Bitboard(0x8080808080808080ULL);

static constexpr Bitboard BB_EMPTY = Bitboard(0ULL);
static constexpr Bitboard BB_FULL = Bitboard(0xFFFFFFFFFFFFFFFFULL);

constexpr Bitboard Bitboard::fromPos(Position pos) {
    return Bitboard(1ULL << pos.getValue());
}

constexpr Bitboard Bitboard::rankOf(uint8_t rank) {
    switch (rank) {
    case 0:
        return BB_RANK_1;
    case 1:
        return BB_RANK_2;
    case 2:
        return BB_RANK_3;
    case 3:
        return BB_RANK_4;
    case 4:
        return BB_RANK_5;
    case 5:
        return BB_RANK_6;
    case 6:
        return BB_RANK_7;
    case 7:
        return BB_RANK_8;
    default:
        assert(0);
    }
}

constexpr Bitboard Bitboard::fileOf(uint8_t file) {
    switch (file) {
    case 0:
        return BB_FILE_A;
    case 1:
        return BB_FILE_B;
    case 2:
        return BB_FILE_C;
    case 3:
        return BB_FILE_D;
    case 4:
        return BB_FILE_E;
    case 5:
        return BB_FILE_F;
    case 6:
        return BB_FILE_G;
    case 7:
        return BB_FILE_H;
    default:
        assert(0);
    }
}

constexpr Bitboard Bitboard::operator<<(uint32_t other) const {
    return Bitboard(value << other);
}

constexpr Bitboard Bitboard::operator>>(uint32_t other) const {
    return Bitboard(value >> other);
}

constexpr Bitboard Bitboard::operator&(const Bitboard& other) const {
    return Bitboard(value & other.value);
}

constexpr Bitboard Bitboard::operator|(const Bitboard& other) const {
    return Bitboard(value | other.value);
}

constexpr Bitboard Bitboard::operator^(const Bitboard& other) const {
    return Bitboard(value ^ other.value);
}

constexpr Bitboard Bitboard::operator~() const {
    return Bitboard(~value);
}

constexpr Bitboard& Bitboard::operator<<=(uint32_t other) {
    value <<= other;
    return *this;
}

constexpr Bitboard& Bitboard::operator>>=(uint32_t other) {
    value >>= other;
    return *this;
}

constexpr Bitboard& Bitboard::operator&=(const Bitboard& other) {
    value &= other.value;
    return *this;
}

constexpr Bitboard& Bitboard::operator|=(const Bitboard& other) {
    value |= other.value;
    return *this;
}

constexpr Bitboard& Bitboard::operator^=(const Bitboard& other) {
    value ^= other.value;
    return *this;
}

constexpr bool Bitboard::hasPos(Position pos) const {
    return value & (1ULL << pos.getValue());
}

constexpr bool Bitboard::hasAny() const {
    return value != 0;
}

constexpr bool Bitboard::empty() const {
    return value == 0;
}

constexpr uint8_t Bitboard::count() const {
#if defined(__GNUC__) || defined(__clang__)
    return static_cast<uint8_t>(__builtin_popcountll(value));
#elif defined(_MSC_VER)
    return static_cast<uint8_t>(__popcnt64(value));
#else
    uint64_t valCpy = value;
    uint8_t count = 0;
    while (valCpy > 0) {
        valCpy &= (valCpy - 1);
        count++;
    }
    return count;
#endif
}

constexpr Position Bitboard::lsb() const {
    assert(value > 0);

#if defined(__GNUC__) || defined(__clang__)
    return Position(__builtin_ctzll(value));
#elif defined(_MSC_VER)
    return Position(__tzcnt_u64(value));
#else
    uint64_t valCpy = value;
    Position pos = 0;
    while ((valCpy & 1) == 0 && valCpy < 64) {
        valCpy >>= 1;
        pos++;
    }
    return Position(pos);
#endif
}

constexpr Bitboard Bitboard::lsbBB() const {
    assert(hasAny());

    return Bitboard(value & -value);
}

constexpr Position Bitboard::poplsb() {
    assert(hasAny());

    Position b = lsb();
    value &= value - 1;
    return b;
}

constexpr Bitboard Bitboard::poplsbBB() {
    assert(hasAny());

    Bitboard res = lsbBB();
    *this &= ~res;

    return res;
}

constexpr Bitboard Bitboard::up(Color perspective) const {
    if (perspective == Color::WHITE)
        return Bitboard(value << 8);
    else
        return Bitboard(value >> 8);
}

constexpr Bitboard Bitboard::down(Color perspective) const {
    if (perspective == Color::WHITE)
        return Bitboard(value >> 8);
    else
        return Bitboard(value << 8);
}

constexpr Bitboard Bitboard::left(Color perspective) const {
    if (perspective == Color::WHITE)
        return Bitboard(value >> 1) & ~BB_FILE_H;
    else
        return Bitboard(value << 1) & ~BB_FILE_A;
}

constexpr Bitboard Bitboard::right(Color perspective) const {
    if (perspective == Color::WHITE)
        return Bitboard(value << 1) & ~BB_FILE_A;
    else
        return Bitboard(value >> 1) & ~BB_FILE_H;
}

constexpr Bitboard Bitboard::flipPerspective() const {
    Bitboard thisCpy = *this;
    Bitboard newVal = BB_EMPTY;

    while (thisCpy.hasAny()) {
        Position pos = thisCpy.poplsb();

        newVal |= fromPos(pos.flipped());
    }

    return newVal;
}

constexpr uint64_t Bitboard::getValue() const {
    return value;
}

inline std::ostream& operator<<(std::ostream& os, const Bitboard& bitboard) {
    std::ostringstream oss;
    for (int rank = 0; rank < 8; rank++) {
        oss << static_cast<char>('1' + rank) << "  ";
        for (int file = 0; file < 8; file++) {
            if (Bitboard mask = Bitboard::fromPos(Position(rank, file));
                (bitboard & mask).hasAny())
                oss << "o";
            else
                oss << ".";

            oss << " ";
        }

        oss << " " << static_cast<char>('1' + rank) << "\n";
    }
    oss << "\n   ";
    for (int file = 0; file < 8; file++) {
        oss << static_cast<char>('a' + file) << " ";
    }

    os << oss.str() << std::flush;

    return os;
}
} // namespace choco
