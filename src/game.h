#pragma once

#include <cstdint>
#include <optional>
#include <vector>

#include "bitboard.h"
#include "types.h"
#include "zobrist.h"

namespace choco {
enum class CastlingSide { KING_SIDE, QUEEN_SIDE };

struct CastlingRights {
    constexpr CastlingRights();

    template<bool allowed>
    constexpr void setCastling(Color color, CastlingSide side);

    template<bool allowed>
    constexpr void setCastling(Color color);

    constexpr bool hasCastlingRights(Color color, CastlingSide side) const;

    constexpr uint8_t getData() const;

    static constexpr Position getCastlingPosition(Color color,
                                                  CastlingSide side);

    // retrieves the from and to of a rook when a king castles
    static constexpr std::pair<Position, Position>
    getRookMovement(Color color, CastlingSide side);

    // retrieves squares that are not allowed to be attacked when castling
    // occurs.
    static constexpr Bitboard getKingThruSquares(Color color,
                                                 CastlingSide side);

    // retrieves squares that are not allowed to be occupied when castling
    // occurs.
    static constexpr Bitboard getRookThruSquares(Color color,
                                                 CastlingSide side);

    static constexpr std::optional<CastlingSide>
    getCastlingSide(Position from, Position to, Color color);

    static constexpr Position getCastleStartingPos(Color color);

    constexpr bool hasAny() const;

    constexpr bool operator==(const CastlingRights& other) const = default;

    constexpr bool operator!=(const CastlingRights& other) const = default;

    friend std::ostream& operator<<(std::ostream& os, const CastlingRights& cr);

  private:
    uint8_t data;
};

struct GameState {
    uint8_t halfMoveClock;
    Position enpassantPos;
    CastlingRights castlingRights;
    Color sideToPlay;

    bool operator==(const GameState& other) const = default;

    bool operator!=(const GameState& other) const = default;

    friend std::ostream& operator<<(std::ostream& os, const CastlingRights& cr);
};

struct CheckInfo {
    CheckInfo();
    CheckInfo(const CheckInfo& other) = default;

    Bitboard checkers = BB_EMPTY;
    Bitboard pinners[2];
    Bitboard discoverers[2];
    Bitboard blockers[2];
};

struct UnmakeMove {
  private:
    UnmakeMove(const GameState& prevState, zobrist::ZobristKey prevZKey,
               Piece pieceTaken, const Move& move, const CheckInfo& checkInfo);

    GameState prevState;
    zobrist::ZobristKey prevZKey;
    Piece pieceTaken;
    Move move;
    CheckInfo checkInfo;

    friend class Game;
};

class PieceColorBoard {
  public:
    PieceColorBoard();
    explicit PieceColorBoard(PieceColorPair data[64]);
    PieceColorBoard(const PieceColorBoard& other) = default;

    PieceColorPair at(Position pos) const;
    void set(Position pos, PieceColorPair pcp);

  private:
    PieceColorPair data[64];
};

class GameRepetition {
  public:
    GameRepetition() = default;

    GameRepetition(const GameRepetition& other) = default;

    bool isRepetition(zobrist::ZobristKey zkey, uint8_t repetitions) const;

    bool isRepetition(zobrist::ZobristKey zkey, uint8_t repetitions,
                      size_t maxLookDistance) const;

    void clear();

    void push(zobrist::ZobristKey zkey);

    void pop();

    bool operator==(const GameRepetition&) const = default;

    bool operator!=(const GameRepetition&) const = default;

  private:
    std::vector<zobrist::ZobristKey> zobStack;
};

class Game {
  public:
    Game();

    Game(const Game& other) = default;

    Game(const PieceColorBoard& pcBoard, const GameState& gameState);

    explicit Game(const std::string& fen);

    Bitboard getBB(Color color, Piece piece) const;

    Bitboard getBB(Color color) const;

    Bitboard getBB() const;

    const GameState& getGameState() const;

    PieceColorPair getPieceColorPairAtPos(Position pos) const;

    template<CastlingSide castlingSide>
    bool canCastle() const;

    bool operator==(const Game& other) const;

    bool operator!=(const Game& other) const;

    UnmakeMove makeMove(const Move& move);

    void unmakeMove(const UnmakeMove& um);

    bool isLegal(const Move& move) const;

    void makeNullMove();

    void unmakeNullMove();

    zobrist::ZobristKey getZKey() const;

    bool isRepetition(uint8_t maxRepetitions) const;

    bool isRepetition(zobrist::ZobristKey repeatingKey,
                      uint8_t maxRepetitions) const;

    bool hasCheckers() const;

    friend std::ostream& operator<<(std::ostream& os, const Game& game);

  private:
    GameState state {};
    Bitboard board[2][6] {};
    Bitboard colorBoard[2] {};

    PieceColorBoard pcBoard;

    zobrist::ZobristKey zkey;

    GameRepetition repetition;

    CheckInfo checkInfo;

    void addPiece(Position pos, Color color, Piece piece);

    void removePiece(Position pos, Color color, Piece piece);

    void movePiece(Position from, Position to, Color color, Piece piece);

    void movePiece(Position from, Position to);

    void updateCheckInfo();

    Bitboard attackersTo(Color attackingColor, Position pos,
                         Bitboard blockers) const;
};

constexpr CastlingRights::CastlingRights() : data(0) {}

template<bool allowed>
constexpr void CastlingRights::setCastling(Color color, CastlingSide side) {
    if constexpr (allowed) {
        data |=
            1 << (static_cast<uint8_t>(color) * 2 + static_cast<uint8_t>(side));
    } else {
        data &= ~(1 << (static_cast<uint8_t>(color) * 2 +
                        static_cast<uint8_t>(side)));
    }
}

template<bool allowed>
constexpr void CastlingRights::setCastling(Color color) {
    setCastling<allowed>(color, CastlingSide::KING_SIDE);
    setCastling<allowed>(color, CastlingSide::QUEEN_SIDE);
}

constexpr bool CastlingRights::hasCastlingRights(Color color,
                                                 CastlingSide side) const {
    return data & 1ULL << (static_cast<uint8_t>(color) * 2 +
                           static_cast<uint8_t>(side));
}

constexpr uint8_t CastlingRights::getData() const {
    return data;
}

constexpr Position CastlingRights::getCastlingPosition(Color color,
                                                       CastlingSide side) {
    constexpr static Position CASTLING_MOVEMENTS[] = {G1, C1, G8, C8};

    return CASTLING_MOVEMENTS[static_cast<uint8_t>(side) +
                              static_cast<uint8_t>(color) * 2];
}

constexpr std::pair<Position, Position>
CastlingRights::getRookMovement(Color color, CastlingSide side) {
    constexpr static std::pair<Position, Position> ROOK_MOVEMENTS[] = {
        {H1, F1}, {A1, D1}, {H8, F8}, {A8, D8}};

    return ROOK_MOVEMENTS[static_cast<uint8_t>(side) +
                          static_cast<uint8_t>(color) * 2];
}

constexpr Bitboard CastlingRights::getKingThruSquares(Color color,
                                                      CastlingSide side) {
    constexpr static Bitboard KING_THRU_SQUARES[] = {
        Bitboard::fromPos(E1) | Bitboard::fromPos(F1) | Bitboard::fromPos(G1),
        Bitboard::fromPos(E1) | Bitboard::fromPos(D1) | Bitboard::fromPos(C1),
        Bitboard::fromPos(E8) | Bitboard::fromPos(F8) | Bitboard::fromPos(G8),
        Bitboard::fromPos(E8) | Bitboard::fromPos(D8) | Bitboard::fromPos(C8)};

    return KING_THRU_SQUARES[static_cast<uint8_t>(side) +
                             static_cast<uint8_t>(color) * 2];
}

constexpr Bitboard CastlingRights::getRookThruSquares(Color color,
                                                      CastlingSide side) {
    constexpr static Bitboard ROOK_THRU_SQUARES[] = {
        Bitboard::fromPos(F1) | Bitboard::fromPos(G1),
        Bitboard::fromPos(D1) | Bitboard::fromPos(C1) | Bitboard::fromPos(B1),
        Bitboard::fromPos(F8) | Bitboard::fromPos(G8),
        Bitboard::fromPos(D8) | Bitboard::fromPos(C8) | Bitboard::fromPos(B8)};

    return ROOK_THRU_SQUARES[static_cast<uint8_t>(side) +
                             static_cast<uint8_t>(color) * 2];
}

constexpr std::optional<CastlingSide>
CastlingRights::getCastlingSide(Position from, Position to, Color color) {
    if (from != getCastleStartingPos(color)) return std::nullopt;

    if (to == getCastlingPosition(color, CastlingSide::KING_SIDE)) {
        return CastlingSide::KING_SIDE;
    }
    if (to == getCastlingPosition(color, CastlingSide::QUEEN_SIDE)) {
        return CastlingSide::QUEEN_SIDE;
    }

    return std::nullopt;
}

constexpr Position CastlingRights::getCastleStartingPos(Color color) {
    return color == Color::WHITE ? E1 : E8;
}

constexpr bool CastlingRights::hasAny() const {
    return data;
}

inline std::ostream& operator<<(std::ostream& os, const CastlingRights& cr) {
    if (cr.hasAny()) {
        if (cr.hasCastlingRights(Color::WHITE, CastlingSide::KING_SIDE))
            os << "K";
        if (cr.hasCastlingRights(Color::WHITE, CastlingSide::QUEEN_SIDE))
            os << "Q";
        if (cr.hasCastlingRights(Color::BLACK, CastlingSide::KING_SIDE))
            os << "k";
        if (cr.hasCastlingRights(Color::BLACK, CastlingSide::QUEEN_SIDE))
            os << "q";
    } else {
        os << "-";
    }
    return os;
}

inline std::ostream& operator<<(std::ostream& os, const GameState& state) {
    os << "Castling: " << state.castlingRights << "\n";

    os << "En passant: ";
    if (state.enpassantPos.isValid()) {
        os << state.enpassantPos;
    } else {
        os << "-";
    }

    os << "\nSide to play: " << state.sideToPlay << "\n";

    os << "Half move clock: " << std::to_string(state.halfMoveClock);

    return os;
}
} // namespace choco
