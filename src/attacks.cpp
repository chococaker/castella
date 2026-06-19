#include "attacks.h"

#include <iostream>
#include <memory>
#include <random>

#include "bitboard.h"
#include "types.h"

namespace choco::attacks {
struct Magic {
    Bitboard mask;
    uint64_t magic;
};

constexpr static uint64_t ROOK_SEED = 459371994;
constexpr static uint64_t BISHOP_SEED = 2595412012;

constexpr static uint8_t ROOK_SHIFTS = 12;
constexpr static size_t ROOK_ATTACKS_SIZE = 1 << ROOK_SHIFTS;
static Magic ROOK_MAGIC[Position::MAX_SQUARE];
static Bitboard ROOK_ATTACKS[Position::MAX_SQUARE][ROOK_ATTACKS_SIZE];

constexpr static uint8_t BISHOP_SHIFTS = 9;
constexpr static size_t BISHOP_ATTACKS_SIZE = 1 << BISHOP_SHIFTS;
static Magic BISHOP_MAGIC[Position::MAX_SQUARE];
static Bitboard BISHOP_ATTACKS[Position::MAX_SQUARE][BISHOP_ATTACKS_SIZE];

static Bitboard KNIGHT_ATTACKS[Position::MAX_SQUARE];
static Bitboard KING_ATTACKS[Position::MAX_SQUARE];

static Bitboard IN_BETWEEN_SQUARES[Position::MAX_SQUARE][Position::MAX_SQUARE];
static Bitboard SIGHTLINES[Position::MAX_SQUARE][Position::MAX_SQUARE];

Bitboard getRay(int rankStep, int fileStep, Bitboard bitboard,
                Position origin) {
    assert(origin.isValid());

    int rank = origin.rank();
    int file = origin.file();

    Bitboard res = BB_EMPTY;
    rank += rankStep;
    file += fileStep;
    while (rank >= 0 && rank < 8 && file >= 0 && file < 8) {
        Position pos = Position(rank, file);
        Bitboard mask = Bitboard::fromPos(pos);
        res |= mask;
        // if the current square is occupied, stop walk
        if ((bitboard & mask).hasAny()) break;
        rank += rankStep;
        file += fileStep;
    }
    return res;
}

template<uint8_t shifts>
constexpr uint16_t getHash(Bitboard bitboard, uint64_t magic) {
    // apply magic, shift right, ensure in-bounds
    return ((bitboard.getValue() * magic) >> (64 - shifts)) &
           ((1ULL << shifts) - 1);
}

Bitboard king(Position pos) {
    assert(pos.isValid());

    return KING_ATTACKS[pos.getValue()];
}

Bitboard queen(Position pos, Bitboard occupied) {
    assert(pos.isValid());

    return bishop(pos, occupied) | rook(pos, occupied);
}

Bitboard bishop(Position pos, Bitboard occupied) {
    assert(pos.isValid());

    auto [mask, magic] = BISHOP_MAGIC[pos.getValue()];
    return BISHOP_ATTACKS[pos.getValue()]
                         [getHash<BISHOP_SHIFTS>(occupied & mask, magic)];
}

Bitboard knight(Position pos) {
    assert(pos.isValid());

    return KNIGHT_ATTACKS[pos.getValue()];
}

Bitboard rook(Position pos, Bitboard occupied) {
    auto [mask, magic] = ROOK_MAGIC[pos.getValue()];
    return ROOK_ATTACKS[pos.getValue()]
                       [getHash<ROOK_SHIFTS>(occupied & mask, magic)];
}

Bitboard pawn(Position pos, Color mover) {
    assert(pos.isValid());

    Bitboard bb = Bitboard::fromPos(pos);
    Bitboard sides = bb.left() | bb.right();

    if (mover == Color::WHITE)
        return sides.up();
    else
        return sides.down();
}

Bitboard pawnAttackersTo(Bitboard pawnBB, Color mover, Bitboard attackedBB) {
    // Simulate the attacked BB as pawns moving in the opposite direction,
    // then AND with the pawns on the board to get the actual pawns
    attackedBB = attackedBB.left() | attackedBB.right();

    if (mover == Color::WHITE)
        attackedBB = attackedBB.down();
    else
        attackedBB = attackedBB.up();

    return attackedBB & pawnBB;
}

template<Piece piece>
Bitboard attacks(Position pos, Bitboard occupied) {
    static_assert(piece != Piece::PAWN);

    switch (piece) {
    case Piece::KING:
        return king(pos);
    case Piece::QUEEN:
        return queen(pos, occupied);
    case Piece::BISHOP:
        return bishop(pos, occupied);
    case Piece::KNIGHT:
        return knight(pos);
    case Piece::ROOK:
        return rook(pos, occupied);

    default:
        assert(0);
    }
}

Bitboard attacks(Piece piece, Position pos, Bitboard occupied) {
    switch (piece) {
    case Piece::KING:
        return king(pos);
    case Piece::QUEEN:
        return queen(pos, occupied);
    case Piece::BISHOP:
        return bishop(pos, occupied);
    case Piece::KNIGHT:
        return knight(pos);
    case Piece::ROOK:
        return rook(pos, occupied);

    default:
        assert(0);
    }
}

// Explicit template instantiations for attacks
template Bitboard attacks<Piece::KING>(Position pos, Bitboard occupied);

template Bitboard attacks<Piece::QUEEN>(Position pos, Bitboard occupied);

template Bitboard attacks<Piece::BISHOP>(Position pos, Bitboard occupied);

template Bitboard attacks<Piece::KNIGHT>(Position pos, Bitboard occupied);

template Bitboard attacks<Piece::ROOK>(Position pos, Bitboard occupied);

Bitboard inBetweenSquares(Position from, Position to) {
    assert(from.isValid());
    assert(to.isValid());
    return IN_BETWEEN_SQUARES[from.getValue()][to.getValue()];
}

bool areAlongLine(Position p1, Position p2, Position p3) {
    return (SIGHTLINES[p1.getValue()][p2.getValue()] & Bitboard::fromPos(p3))
        .hasAny();
}

Bitboard getBishopBlockerMask(Position pos) {
    Bitboard rays = getRay(1, 1, BB_EMPTY, pos) | getRay(1, -1, BB_EMPTY, pos) |
                    getRay(-1, 1, BB_EMPTY, pos) |
                    getRay(-1, -1, BB_EMPTY, pos);

    rays &= ~BB_FILE_A;
    rays &= ~BB_FILE_H;
    rays &= ~BB_RANK_1;
    rays &= ~BB_RANK_8;

    rays &= ~Bitboard::fromPos(pos);

    return rays;
}

Bitboard getRookBlockerMask(Position pos) {
    // retrieve attack rays
    Bitboard rankMask = Bitboard::rankOf(pos.rank());
    Bitboard fileMask = Bitboard::fileOf(pos.file());
    Bitboard rays = (rankMask | fileMask);

    // remove edges UNLESS rook is on the edge (for magics)
    if (fileMask != BB_FILE_A) rays &= ~BB_FILE_A;
    if (fileMask != BB_FILE_H) rays &= ~BB_FILE_H;
    if (rankMask != BB_RANK_1) rays &= ~BB_RANK_1;
    if (rankMask != BB_RANK_8) rays &= ~BB_RANK_8;

    // exclude the location of the rook
    rays &= ~Bitboard::fromPos(pos);

    return rays;
}

template<Piece piece>
void populateMagic() {
    using enum Piece;

    static_assert(piece == ROOK || piece == BISHOP);

    constexpr int SEED = (piece == ROOK) ? ROOK_SEED : BISHOP_SEED;
    Magic* magicArr = (piece == ROOK) ? ROOK_MAGIC : BISHOP_MAGIC;
    constexpr uint8_t shifts = (piece == ROOK) ? ROOK_SHIFTS : BISHOP_SHIFTS;
    constexpr auto getBlockerMask =
        (piece == ROOK) ? getRookBlockerMask : getBishopBlockerMask;
    const auto& attacks = [] {
        if constexpr (piece == ROOK) {
            return ROOK_ATTACKS;
        } else if constexpr (piece == BISHOP) {
            return BISHOP_ATTACKS;
        }
    }();

    std::mt19937_64 magicGenerator(SEED); // NOLINT cert-msc51-cpp

    // enumerate through all squares
    for (int i = 0; i < Position::MAX_SQUARE; i++) {
        Position pos = Position(i);
        Bitboard blockers = getBlockerMask(Position(i));
        magicArr[i].mask = blockers;

        while (true) {
            // xor 3x to keep complexity low (i
            uint64_t magic =
                magicGenerator() & magicGenerator() & magicGenerator();
            // reset from previous attempt (garbage)
            std::fill(std::begin(attacks[i]), std::end(attacks[i]), BB_EMPTY);

            bool isValidHash = true;

            // enumerate through all permutations of blocker (Carry-Ripple)
            Bitboard permut = BB_EMPTY;
            do {
                Bitboard attackBoard = {};
                if constexpr (piece == ROOK) {
                    // side to side
                    attackBoard =
                        getRay(1, 0, permut, pos) | getRay(-1, 0, permut, pos) |
                        getRay(0, 1, permut, pos) | getRay(0, -1, permut, pos);
                } else { // bishop
                    // diagonal
                    attackBoard = getRay(1, 1, permut, pos) |
                                  getRay(1, -1, permut, pos) |
                                  getRay(-1, 1, permut, pos) |
                                  getRay(-1, -1, permut, pos);
                }

                uint16_t hash = getHash<shifts>(permut, magic);

                // hash collision detected, break loop
                if (attacks[i][hash].hasAny() &&
                    attacks[i][hash] != attackBoard) {
                    isValidHash = false;
                    break;
                }

                attacks[i][hash] = attackBoard;

                // move onto next permutation
                permut = Bitboard(permut.getValue() - blockers.getValue()) &
                         blockers;
            } while (permut.hasAny());

            // valid hash found, move on!
            if (isValidHash) {
                magicArr[i].magic = magic;
                break;
            }
        }
    }
}

template<Piece pieceType>
void populateAttacks();

template<>
void populateAttacks<Piece::KING>() {
    for (size_t i = 0; i < Position::MAX_SQUARE; i++) {
        Position pos = Position(i);

        Bitboard attacks = BB_EMPTY;
        Bitboard posBB = Bitboard::fromPos(pos);

        // x x x
        // x o x
        // x x x
        attacks |= posBB.up().left();
        attacks |= posBB.up();
        attacks |= posBB.up().right();
        attacks |= posBB.right();
        attacks |= posBB.down().right();
        attacks |= posBB.down();
        attacks |= posBB.down().left();
        attacks |= posBB.left();

        KING_ATTACKS[i] = attacks;
    }
}

template<>
void populateAttacks<Piece::KNIGHT>() {
    for (int i = 0; i < Position::MAX_SQUARE; i++) {
        Position pos = Position(i);

        Bitboard attacks = BB_EMPTY;
        Bitboard posBB = Bitboard::fromPos(pos);

        //  x x
        // x   x
        //   o
        // x   x
        //  x x
        attacks |= posBB.up().up().right();
        attacks |= posBB.up().up().left();
        attacks |= posBB.up().left().left();
        attacks |= posBB.down().left().left();
        attacks |= posBB.down().down().left();
        attacks |= posBB.down().down().right();
        attacks |= posBB.down().right().right();
        attacks |= posBB.up().right().right();

        KNIGHT_ATTACKS[i] = attacks;
    }
}

void initInBetweenSquares() {
    for (int i = 0; i < Position::MAX_SQUARE; i++) {
        Position src = Position(i);
        for (int j = 0; j < Position::MAX_SQUARE; j++) {
            Position dest = Position(j);

            if (src == dest) continue;

            int rankDiff =
                static_cast<int>(src.rank()) - static_cast<int>(dest.rank());
            int fileDiff =
                static_cast<int>(src.file()) - static_cast<int>(dest.file());

            Bitboard& bbToChange =
                IN_BETWEEN_SQUARES[src.getValue()][dest.getValue()];

            if (src.rank() == dest.rank()) { // horizontal
                bbToChange = getRay(0, src.file() > dest.file() ? -1 : 1,
                                    Bitboard::fromPos(dest), src);
            } else if (src.file() == dest.file()) { // vertical
                bbToChange = getRay(src.rank() > dest.rank() ? -1 : 1, 0,
                                    Bitboard::fromPos(dest), src);
            } else if (std::abs(rankDiff) == std::abs(fileDiff)) { // diagonal
                int rankStep = rankDiff > 0 ? -1 : 1;
                int fileStep = fileDiff > 0 ? -1 : 1;
                bbToChange =
                    getRay(rankStep, fileStep, Bitboard::fromPos(dest), src);
            } else {
                bbToChange = BB_EMPTY;
            }

            // remove destination square
            bbToChange &= ~Bitboard::fromPos(dest);
        }
    }
}

void initSightlines() {
    for (int i = 0; i < Position::MAX_SQUARE; i++) {
        Position src = Position(i);
        for (int j = 0; j < Position::MAX_SQUARE; j++) {
            Position dest = Position(j);
            int rankDiff = src.rank() - dest.rank();
            int fileDiff = src.file() - dest.file();

            Bitboard& bbToChange = SIGHTLINES[src.getValue()][dest.getValue()];

            if (rankDiff == 0) {
                Position rayOrigin = Position(src.rank(), 0);
                bbToChange = getRay(0, 1, BB_EMPTY, rayOrigin) |
                             Bitboard::fromPos(rayOrigin);
            } else if (fileDiff == 0) {
                Position rayOrigin = Position(0, src.file());
                bbToChange = getRay(1, 0, BB_EMPTY, rayOrigin) |
                             Bitboard::fromPos(rayOrigin);
            } else if (std::abs(rankDiff) == std::abs(fileDiff)) {
                int rankStep = rankDiff > 0 ? -1 : 1;
                int fileStep = fileDiff > 0 ? -1 : 1;
                bbToChange = BB_EMPTY;
                bbToChange |= getRay(rankStep, fileStep, BB_EMPTY, src);
                bbToChange |= getRay(-rankStep, -fileStep, BB_EMPTY, src);
                bbToChange |= Bitboard::fromPos(src);
            } else {
                bbToChange = BB_EMPTY;
            }
        }
    }
}

void initAttacks() {
    populateAttacks<Piece::KING>();
    populateAttacks<Piece::KNIGHT>();
    populateMagic<Piece::BISHOP>();
    populateMagic<Piece::ROOK>();

    initInBetweenSquares();
    initSightlines();
}
} // namespace choco::attacks
