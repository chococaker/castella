#pragma once

#include <cassert>
#include <cstdint>
#include <ostream>

namespace choco {
enum class Color : uint8_t { WHITE, BLACK };

std::ostream& operator<<(std::ostream& os, const Color& color);

constexpr Color operator~(const Color& c) {
    return static_cast<Color>(static_cast<int>(c) ^ 1);
}

enum class Piece : uint8_t { KING, QUEEN, BISHOP, KNIGHT, ROOK, PAWN, INVALID };

enum class PieceColorPair : uint8_t {
    WHITE_KING,
    WHITE_QUEEN,
    WHITE_BISHOP,
    WHITE_KNIGHT,
    WHITE_ROOK,
    WHITE_PAWN,

    BLACK_KING,
    BLACK_QUEEN,
    BLACK_BISHOP,
    BLACK_KNIGHT,
    BLACK_ROOK,
    BLACK_PAWN,

    INVALID
};

constexpr PieceColorPair toPieceColorPair(Piece piece, Color color) {
    if (piece == Piece::INVALID) return PieceColorPair::INVALID;
    return static_cast<PieceColorPair>(static_cast<int>(piece) +
                                       static_cast<int>(color) * 6);
}

constexpr Piece getPieceOf(PieceColorPair pieceColorPair) {
    if (pieceColorPair == PieceColorPair::INVALID) return Piece::INVALID;

    return static_cast<Piece>(static_cast<int>(pieceColorPair) % 6);
}

constexpr Color getColorOf(PieceColorPair pieceColorPair) {
    assert(pieceColorPair != PieceColorPair::INVALID);

    return static_cast<Color>(static_cast<int>(pieceColorPair) > 5);
}

std::ostream& operator<<(std::ostream& os, const Piece& piece);

enum class Direction : int8_t { UP = 8, DOWN = -UP, LEFT = -1, RIGHT = -LEFT };

class Position {
  public:
    static constexpr uint8_t MAX_SQUARE = 64;

    constexpr Position() = default;

    explicit constexpr Position(uint8_t value);

    constexpr Position(uint8_t rank, uint8_t file);

    constexpr uint8_t rank() const;

    constexpr uint8_t file() const;

    constexpr Position flipped() const;

    constexpr bool operator==(const Position& other) const = default;

    constexpr bool operator!=(const Position& other) const = default;

    constexpr Position& operator+=(uint8_t val);

    constexpr Position& operator-=(uint8_t val);

    constexpr Position& operator++();

    constexpr Position operator++(int);

    constexpr Position& operator--();

    constexpr Position operator--(int);

    constexpr Position operator+(uint8_t val) const;

    constexpr Position operator-(uint8_t val) const;

    constexpr uint8_t getValue() const;

    constexpr bool isValid() const;

    friend std::ostream& operator<<(std::ostream& os, const Position& pos);

  private:
    uint8_t value;
};

struct Move {
    constexpr Move();

    constexpr Move(Piece pieceType, Position from, Position to);

    constexpr Move(Piece pieceType, Position from, Position to,
                   Piece promotionType);

    Piece pieceType;
    Position from;
    Position to;
    Piece promotionType;

    constexpr bool operator==(const Move& other) const;

    constexpr bool isValid() const;

    friend std::ostream& operator<<(std::ostream& os, const Move& move);
};

class MoveList {
  public:
    constexpr MoveList();

    constexpr Move& operator[](size_t i);

    constexpr const Move& operator[](size_t i) const;

    constexpr uint8_t size() const;

    constexpr Move pop();

    constexpr void push_back(Move&& move);

    constexpr void push_back(const Move& move);

    constexpr void swap(uint8_t n1, uint8_t n2);

    constexpr bool contains(const Move& move) const;

    class Iterator {
      public:
        constexpr explicit Iterator(Move* ptr) : ptr(ptr) {}

        constexpr Move& operator*() {
            return *ptr;
        }
        constexpr const Move& operator*() const {
            return *ptr;
        }

        constexpr Move* operator->() {
            return ptr;
        }
        constexpr const Move* operator->() const {
            return ptr;
        }

        constexpr Iterator& operator++() {
            ++ptr;
            return *this;
        }

        constexpr Iterator operator++(int) {
            Iterator temp = *this;
            ++ptr;
            return temp;
        }

        constexpr Iterator& operator--() {
            --ptr;
            return *this;
        }

        constexpr Iterator operator--(int) {
            Iterator temp = *this;
            --ptr;
            return temp;
        }

        constexpr Iterator operator+(size_t n) const {
            return Iterator(ptr + n);
        }

        constexpr Iterator operator-(size_t n) const {
            return Iterator(ptr - n);
        }

        constexpr size_t operator-(const Iterator& other) const {
            return ptr - other.ptr;
        }

        constexpr bool operator==(const Iterator& other) const {
            return ptr == other.ptr;
        }

        constexpr bool operator!=(const Iterator& other) const {
            return ptr != other.ptr;
        }

      private:
        Move* ptr;
    };

    constexpr Iterator begin();

    constexpr Iterator end();

    friend std::ostream& operator<<(std::ostream& os, const MoveList& moves);

  private:
    static constexpr size_t MAX_MOVE_COUNT = 218;
    Move moves[MAX_MOVE_COUNT];
    uint8_t n;
};

/**
 * Both in terms of white
 */
struct UntaperedEval {
    int32_t mg;
    int32_t eg;

    UntaperedEval operator+(const UntaperedEval& other) const;
};

constexpr Direction operator+(Direction d1, Direction d2) {
    return static_cast<Direction>(static_cast<int>(d1) + static_cast<int>(d2));
}

constexpr Direction operator*(int i, Direction d) {
    return static_cast<Direction>(i * static_cast<int>(d));
}

constexpr Position::Position(uint8_t value) : value(value) {}

static constexpr Position A1 = Position(0);
static constexpr Position B1 = Position(1);
static constexpr Position C1 = Position(2);
static constexpr Position D1 = Position(3);
static constexpr Position E1 = Position(4);
static constexpr Position F1 = Position(5);
static constexpr Position G1 = Position(6);
static constexpr Position H1 = Position(7);

static constexpr Position A2 = Position(8);
static constexpr Position B2 = Position(9);
static constexpr Position C2 = Position(10);
static constexpr Position D2 = Position(11);
static constexpr Position E2 = Position(12);
static constexpr Position F2 = Position(13);
static constexpr Position G2 = Position(14);
static constexpr Position H2 = Position(15);

static constexpr Position A3 = Position(16);
static constexpr Position B3 = Position(17);
static constexpr Position C3 = Position(18);
static constexpr Position D3 = Position(19);
static constexpr Position E3 = Position(20);
static constexpr Position F3 = Position(21);
static constexpr Position G3 = Position(22);
static constexpr Position H3 = Position(23);

static constexpr Position A4 = Position(24);
static constexpr Position B4 = Position(25);
static constexpr Position C4 = Position(26);
static constexpr Position D4 = Position(27);
static constexpr Position E4 = Position(28);
static constexpr Position F4 = Position(29);
static constexpr Position G4 = Position(30);
static constexpr Position H4 = Position(31);

static constexpr Position A5 = Position(32);
static constexpr Position B5 = Position(33);
static constexpr Position C5 = Position(34);
static constexpr Position D5 = Position(35);
static constexpr Position E5 = Position(36);
static constexpr Position F5 = Position(37);
static constexpr Position G5 = Position(38);
static constexpr Position H5 = Position(39);

static constexpr Position A6 = Position(40);
static constexpr Position B6 = Position(41);
static constexpr Position C6 = Position(42);
static constexpr Position D6 = Position(43);
static constexpr Position E6 = Position(44);
static constexpr Position F6 = Position(45);
static constexpr Position G6 = Position(46);
static constexpr Position H6 = Position(47);

static constexpr Position A7 = Position(48);
static constexpr Position B7 = Position(49);
static constexpr Position C7 = Position(50);
static constexpr Position D7 = Position(51);
static constexpr Position E7 = Position(52);
static constexpr Position F7 = Position(53);
static constexpr Position G7 = Position(54);
static constexpr Position H7 = Position(55);

static constexpr Position A8 = Position(56);
static constexpr Position B8 = Position(57);
static constexpr Position C8 = Position(58);
static constexpr Position D8 = Position(59);
static constexpr Position E8 = Position(60);
static constexpr Position F8 = Position(61);
static constexpr Position G8 = Position(62);
static constexpr Position H8 = Position(63);

static constexpr Position INVALID_POSITION = Position(Position::MAX_SQUARE);

constexpr Position::Position(uint8_t rank, uint8_t file)
    : value((rank * 8) + file) {}

constexpr uint8_t Position::rank() const {
    assert(isValid());

    return value / 8;
}

constexpr uint8_t Position::file() const {
    assert(isValid());

    return value % 8;
}

constexpr Position Position::flipped() const {
    assert(isValid());

    return {static_cast<uint8_t>(7 - rank()), file()};
}

constexpr Position& Position::operator+=(uint8_t val) {
    *this = *this + val;
    return *this;
}

constexpr Position& Position::operator-=(uint8_t val) {
    *this = *this - val;
    return *this;
}

constexpr Position& Position::operator++() {
    value++;
    return *this;
}

constexpr Position Position::operator++(int) {
    Position tmp = *this;
    operator++();
    return tmp;
}

constexpr Position& Position::operator--() {
    value--;
    return *this;
}

constexpr Position Position::operator--(int) {
    Position tmp = *this;
    operator--();
    return tmp;
}

constexpr Position Position::operator+(uint8_t val) const {
    return Position(value + val);
}

constexpr Position Position::operator-(uint8_t val) const {
    return Position(value - val);
}

constexpr uint8_t Position::getValue() const {
    return value;
}

constexpr bool Position::isValid() const {
    return value < MAX_SQUARE;
}

constexpr Position operator+(Position pos, Direction dir) {
    return Position(pos.getValue() + static_cast<int>(dir));
}

constexpr Position operator-(Position pos, Direction dir) {
    return Position(pos.getValue() - static_cast<int>(dir));
}

constexpr Position& operator+=(Position& pos, Direction d) {
    return pos = pos + d;
}

constexpr Position& operator-=(Position& pos, Direction d) {
    return pos = pos - d;
}

constexpr Move::Move()
    : pieceType(Piece::INVALID), from(INVALID_POSITION), to(INVALID_POSITION),
      promotionType(Piece::INVALID) {}

static constexpr Move INVALID_MOVE = Move();

constexpr Move::Move(Piece pieceType, Position from, Position to)
    : pieceType(pieceType), from(from), to(to), promotionType(Piece::INVALID) {}

constexpr Move::Move(Piece pieceType, Position from, Position to,
                     Piece promotionType)
    : pieceType(pieceType), from(from), to(to), promotionType(promotionType) {}

constexpr bool Move::operator==(const Move& other) const {
    return pieceType == other.pieceType && from == other.from &&
           to == other.to && promotionType == other.promotionType;
}

constexpr bool Move::isValid() const {
    return pieceType != Piece::INVALID && from != to &&
           from != INVALID_POSITION && to != INVALID_POSITION &&
           promotionType != Piece::PAWN && promotionType != Piece::KING;
}

constexpr MoveList::MoveList() : moves {}, n(0) {}

constexpr Move& MoveList::operator[](size_t i) {
    assert(i < n);
    return moves[i];
}

constexpr const Move& MoveList::operator[](size_t i) const {
    assert(i < n);
    return moves[i];
}

constexpr uint8_t MoveList::size() const {
    return n;
}

constexpr Move MoveList::pop() {
    assert(n > 0);

    return moves[--n];
}

constexpr void MoveList::push_back(Move&& move) {
    assert(n + 1 < MAX_MOVE_COUNT);

    moves[n++] = move;
}

constexpr void MoveList::push_back(const Move& move) {
    assert(n + 1 < MAX_MOVE_COUNT);

    moves[n++] = move;
}

constexpr void MoveList::swap(uint8_t n1, uint8_t n2) {
    assert(n1 < size() && n2 < size());

    std::swap(moves[n1], moves[n2]);
}

constexpr bool MoveList::contains(const Move& move) const {
    for (int i = 0; i < n; i++) {
        if (operator[](i) == move) return true;
    }

    return false;
}

constexpr MoveList::Iterator MoveList::begin() {
    return Iterator(moves);
}

constexpr MoveList::Iterator MoveList::end() {
    return Iterator(moves + n);
}

inline UntaperedEval UntaperedEval::operator+(const UntaperedEval& other) const {
    return { mg + other.mg, eg + other.eg };
}
} // namespace choco
