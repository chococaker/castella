#include "game.h"

#include <algorithm>
#include <iostream>

#include "attacks.h"
#include "str_util.h"
#include "uci.h"
#include "zobrist.h"

namespace choco {
namespace {
std::pair<PieceColorBoard, GameState> dataFromFen(const std::string& fen) {
    GameState state;
    PieceColorBoard board {};

    std::vector<std::string> fenParts = util::split(fen, " ");

    auto addPiece = [&](Position pos, Color color, Piece piece) {
        board.set(pos, toPieceColorPair(piece, color));
    };

    // piece positions
    std::vector<std::string> positions = util::split(fenParts[0], "/");
    for (int rank = 0; rank < 8; rank++) {
        const std::string& row = positions[7 - rank];
        int file = 0;

        for (size_t strIndex = 0; strIndex < row.length() && file < 8;
             strIndex++) {
            Position pos = Position(rank, file);
            switch (row[strIndex]) {
            case 'K':
                addPiece(pos, Color::WHITE, Piece::KING);
                break;
            case 'Q':
                addPiece(pos, Color::WHITE, Piece::QUEEN);
                break;
            case 'B':
                addPiece(pos, Color::WHITE, Piece::BISHOP);
                break;
            case 'N':
                addPiece(pos, Color::WHITE, Piece::KNIGHT);
                break;
            case 'R':
                addPiece(pos, Color::WHITE, Piece::ROOK);
                break;
            case 'P':
                addPiece(pos, Color::WHITE, Piece::PAWN);
                break;

            case 'k':
                addPiece(pos, Color::BLACK, Piece::KING);
                break;
            case 'q':
                addPiece(pos, Color::BLACK, Piece::QUEEN);
                break;
            case 'b':
                addPiece(pos, Color::BLACK, Piece::BISHOP);
                break;
            case 'n':
                addPiece(pos, Color::BLACK, Piece::KNIGHT);
                break;
            case 'r':
                addPiece(pos, Color::BLACK, Piece::ROOK);
                break;
            case 'p':
                addPiece(pos, Color::BLACK, Piece::PAWN);
                break;

            default: // numeric
                int val = row[strIndex] - '0';
                file += val - 1;
                break;
            }
            file++;
        }
    }

    // turn
    state.sideToPlay = (fenParts[1] == "w") ? Color::WHITE : Color::BLACK;

    // castling
    const std::string& castlingString = fenParts[2];
    if (castlingString.find('K') != std::string::npos)
        state.castlingRights.setCastling<true>(Color::WHITE,
                                               CastlingSide::KING_SIDE);
    if (castlingString.find('Q') != std::string::npos)
        state.castlingRights.setCastling<true>(Color::WHITE,
                                               CastlingSide::QUEEN_SIDE);
    if (castlingString.find('k') != std::string::npos)
        state.castlingRights.setCastling<true>(Color::BLACK,
                                               CastlingSide::KING_SIDE);
    if (castlingString.find('q') != std::string::npos)
        state.castlingRights.setCastling<true>(Color::BLACK,
                                               CastlingSide::QUEEN_SIDE);

    // en passant
    if (const std::string& passantString = fenParts[3];
        passantString.length() == 2) {
        uint8_t passantFile = passantString[0] - 'a';
        uint8_t passantRank = passantString[1] - '0' - 1;
        state.enpassantPos = Position(passantRank, passantFile);
    } else {
        state.enpassantPos = INVALID_POSITION;
    }

    // half moves
    state.halfMoveClock = (fenParts.size() >= 5 && !fenParts[4].empty())
                              ? std::stoi(fenParts[4])
                              : 0;

    return std::make_pair(board, state);
}
} // namespace

CheckInfo::CheckInfo()
    : pinners {BB_EMPTY, BB_EMPTY}, discoverers {BB_EMPTY, BB_EMPTY},
      blockers {BB_EMPTY, BB_EMPTY} {}

UnmakeMove::UnmakeMove(const GameState& prevState, zobrist::ZobristKey prevZKey,
                       Piece pieceTaken, const Move& move,
                       const CheckInfo& checkInfo)
    : prevState(prevState), prevZKey(prevZKey), pieceTaken(pieceTaken),
      move(move), checkInfo(checkInfo) {}

PieceColorBoard::PieceColorBoard() : data() {
    std::fill_n(data, 64, PieceColorPair::INVALID);
}

PieceColorBoard::PieceColorBoard(PieceColorPair data[64]) : data() {
    std::copy_n(this->data, 64, data);
}

PieceColorPair PieceColorBoard::at(Position pos) const {
    assert(pos.isValid());

    return data[pos.getValue()];
}

void PieceColorBoard::set(Position pos, PieceColorPair pcp) {
    assert(pos.isValid());

    data[pos.getValue()] = pcp;
}

bool GameRepetition::isRepetition(zobrist::ZobristKey zkey,
                                  uint8_t repetitions) const {
    uint8_t c = 0;

    for (int i = static_cast<int>(zobStack.size()) - 1; i >= 0; i--) {
        if (zobStack[i] == zkey) c++;
        if (c == repetitions) return true;
    }

    return false;
}

bool GameRepetition::isRepetition(zobrist::ZobristKey zkey, uint8_t repetitions,
                                  size_t maxLookDistance) const {
    uint8_t c = 0;

    maxLookDistance =
        std::clamp(maxLookDistance, static_cast<size_t>(0), zobStack.size());

    int finalIndex = zobStack.size() - maxLookDistance;

    for (int i = static_cast<int>(zobStack.size()) - 1; i >= finalIndex; i--) {
        assert(i >= 0);
        if (zobStack[i] == zkey) c++;
        if (c == repetitions) return true;
    }

    return false;
}

void GameRepetition::clear() {
    zobStack.clear();
}

void GameRepetition::push(zobrist::ZobristKey zkey) {
    zobStack.emplace_back(zkey);
}

void GameRepetition::pop() {
    zobStack.pop_back();
}

Game::Game() : Game(STARTING_FEN) {}

Game::Game(const PieceColorBoard& pcBoard, const GameState& gameState) {
    std::fill_n(&this->board[0][0], 2 * 6, BB_EMPTY);

    colorBoard[0] = BB_EMPTY;
    colorBoard[1] = BB_EMPTY;

    for (int i = 0; i < 64; i++) {
        Position pos = Position(i);
        PieceColorPair pcp = pcBoard.at(pos);
        if (pcp != PieceColorPair::INVALID) {
            addPiece(pos, getColorOf(pcp), getPieceOf(pcp));
        }
    }

    this->state = gameState;
    this->zkey = zobrist::ZobristKey(*this);

    repetition = GameRepetition();
    repetition.push(getZKey());

    updateCheckInfo();
}

Game::Game(const std::string& fen)
    : Game(std::make_from_tuple<Game>(dataFromFen(fen))) {}

Bitboard Game::getBB(Color color, Piece piece) const {
    return board[static_cast<size_t>(color)][static_cast<size_t>(piece)];
}

Bitboard Game::getBB(Color color) const {
    assert(colorBoard[static_cast<size_t>(color)] ==
           (getBB(color, Piece::KING) | getBB(color, Piece::QUEEN) |
            getBB(color, Piece::BISHOP) | getBB(color, Piece::KNIGHT) |
            getBB(color, Piece::ROOK) | getBB(color, Piece::PAWN)));

    return colorBoard[static_cast<size_t>(color)];
}

Bitboard Game::getBB() const {
    return getBB(Color::WHITE) | getBB(Color::BLACK);
}

const GameState& Game::getGameState() const {
    return state;
}

PieceColorPair Game::getPieceColorPairAtPos(Position pos) const {
    assert(pos.isValid());

    return pcBoard.at(pos);
}

template<CastlingSide castlingSide>
bool Game::canCastle() const {
    return state.castlingRights.hasCastlingRights(getGameState().sideToPlay,
                                                  castlingSide) &&
           !(getBB() & CastlingRights::getRookThruSquares(
                           getGameState().sideToPlay, castlingSide))
                .hasAny();
}

bool Game::operator==(const Game& other) const {
    for (uint8_t i = 0; i < 6; i++) {
        if (Piece piece = static_cast<Piece>(i);
            getBB(Color::WHITE, piece) != other.getBB(Color::WHITE, piece) ||
            getBB(Color::BLACK, piece) != other.getBB(Color::BLACK, piece))
            return false;
    }

    assert(getBB() == other.getBB());

    return state == other.state && repetition == other.repetition;
}

bool Game::operator!=(const Game& other) const {
    return !(*this == other);
}

UnmakeMove Game::makeMove(const Move& move) {
    bool incHFMClock = true;

    zobrist::ZobristKey origZKey = getZKey();

    PieceColorPair pieceColorPair = getPieceColorPairAtPos(move.to);

    Piece capturedPiece = getPieceOf(pieceColorPair);

    assert(capturedPiece != Piece::KING);

    if (capturedPiece != Piece::INVALID) {
        removePiece(move.to, ~state.sideToPlay, capturedPiece);
        incHFMClock = false;
    }
    movePiece(move.from, move.to, state.sideToPlay, move.pieceType);

    UnmakeMove um = UnmakeMove(state, origZKey, capturedPiece, move, checkInfo);

    /*** PROMOTION ***/
    if (move.promotionType != Piece::INVALID) {
        removePiece(move.to, state.sideToPlay, Piece::PAWN);
        addPiece(move.to, state.sideToPlay, move.promotionType);
    }

    /** EN PASSANT ***/
    bool resetEnpassant = true;
    // special pawn cases
    if (move.pieceType == Piece::PAWN) {
        // doing en passant
        if (move.to == state.enpassantPos) {
            if (state.sideToPlay == Color::WHITE)
                removePiece(move.to + Direction::DOWN, Color::BLACK,
                            Piece::PAWN);
            else
                removePiece(move.to + Direction::UP, Color::WHITE, Piece::PAWN);
        }

        // double pushing (register en passant)
        bool didDoublePush =
            (state.sideToPlay == Color::WHITE
                 ? move.to.getValue() - move.from.getValue()
                 : move.from.getValue() - move.to.getValue()) == 16;
        if (didDoublePush) {
            if (state.enpassantPos.isValid()) {
                zkey.updateEnPassant(state.enpassantPos.file());
            }

            state.enpassantPos = state.sideToPlay == Color::WHITE
                                     ? move.to + Direction::DOWN
                                     : move.to + Direction::UP;
            resetEnpassant = false;

            zkey.updateEnPassant(state.enpassantPos.file());
        }

        incHFMClock = false;
    }

    /*** CASTLING MANAGEMENT ***/
    // remove castling data from zkey
    zkey.updateCastling(state.castlingRights);

    if (move.pieceType == Piece::KING) {
        // disable castling
        state.castlingRights.setCastling<false>(state.sideToPlay);

        if (std::optional<CastlingSide> castlingSide =
                CastlingRights::getCastlingSide(move.from, move.to,
                                                state.sideToPlay);
            castlingSide.has_value()) {
            auto [rookFrom, rookTo] = CastlingRights::getRookMovement(
                state.sideToPlay, castlingSide.value());
            movePiece(rookFrom, rookTo, state.sideToPlay, Piece::ROOK);
        }
    }

    // disable castling upon rook movement
    if (move.pieceType == Piece::ROOK) {
        if (move.from == CastlingRights::getRookMovement(
                             state.sideToPlay, CastlingSide::KING_SIDE)
                             .first)
            state.castlingRights.setCastling<false>(state.sideToPlay,
                                                    CastlingSide::KING_SIDE);
        if (move.from == CastlingRights::getRookMovement(
                             state.sideToPlay, CastlingSide::QUEEN_SIDE)
                             .first)
            state.castlingRights.setCastling<false>(state.sideToPlay,
                                                    CastlingSide::QUEEN_SIDE);
    }

    // disable castling upon rook capture
    if (capturedPiece == Piece::ROOK) {
        if (move.to == CastlingRights::getRookMovement(~state.sideToPlay,
                                                       CastlingSide::KING_SIDE)
                           .first)
            state.castlingRights.setCastling<false>(~state.sideToPlay,
                                                    CastlingSide::KING_SIDE);
        if (move.to == CastlingRights::getRookMovement(~state.sideToPlay,
                                                       CastlingSide::QUEEN_SIDE)
                           .first)
            state.castlingRights.setCastling<false>(~state.sideToPlay,
                                                    CastlingSide::QUEEN_SIDE);
    }

    // add back castling data to zkey
    zkey.updateCastling(state.castlingRights);

    /*** EN PASSANT PT. 2 ***/
    // reset en passant
    if (resetEnpassant) {
        if (state.enpassantPos.isValid()) {
            zkey.updateEnPassant(state.enpassantPos.file());
        }
        state.enpassantPos = INVALID_POSITION;
    }

    /*** NEXT-MOVE MANAGEMENT ***/
    state.sideToPlay = ~state.sideToPlay; // interim side-switch
    zkey.flipSideToMove();

    if (incHFMClock) {
        state.halfMoveClock++;
    } else {
        state.halfMoveClock = 0;
    }

    repetition.push(getZKey());

    updateCheckInfo();

    return um;
}

void Game::unmakeMove(const UnmakeMove& um) {
    repetition.pop();

    state = um.prevState;

    if (um.move.promotionType == Piece::INVALID) {
        movePiece(um.move.to, um.move.from, state.sideToPlay,
                  um.move.pieceType);
    } else {
        addPiece(um.move.from, state.sideToPlay, um.move.pieceType);
        removePiece(um.move.to, state.sideToPlay, um.move.promotionType);
    }

    // piece captures
    if (um.move.pieceType == Piece::PAWN && um.move.to == state.enpassantPos) {
        if (state.sideToPlay == Color::WHITE)
            addPiece(um.move.to + Direction::DOWN, Color::BLACK, Piece::PAWN);
        else
            addPiece(um.move.to + Direction::UP, Color::WHITE, Piece::PAWN);
    } else if (um.pieceTaken != Piece::INVALID) {
        addPiece(um.move.to, ~state.sideToPlay, um.pieceTaken);
    }

    // un-castling
    if (um.move.pieceType == Piece::KING) {
        if (std::optional<CastlingSide> castlingSide =
                CastlingRights::getCastlingSide(um.move.from, um.move.to,
                                                state.sideToPlay);
            castlingSide.has_value()) {
            auto [origin, to] = CastlingRights::getRookMovement(
                state.sideToPlay, castlingSide.value());
            movePiece(to, origin, state.sideToPlay, Piece::ROOK);
        }
    }

    this->checkInfo = um.checkInfo;

    this->zkey = um.prevZKey;
}

bool Game::isLegal(const Move& move) const {
    Position kingPos = getBB(state.sideToPlay, Piece::KING).lsb();

    // en passant
    if (move.pieceType == Piece::PAWN && move.to == state.enpassantPos) {
        Position captureSq =
            move.to + (state.sideToPlay == Color::WHITE ? -8 : 8);
        Bitboard piecesAfterMove = Bitboard::fromPos(move.to) |
                                   (getBB() ^ Bitboard::fromPos(move.from) ^
                                    Bitboard::fromPos(captureSq));
        Bitboard queenBB = getBB(~state.sideToPlay, Piece::QUEEN);
        Bitboard rookSliderBB = getBB(~state.sideToPlay, Piece::ROOK) | queenBB;
        Bitboard diagSliderBB =
            getBB(~state.sideToPlay, Piece::BISHOP) | queenBB;

        return (attacks::rook(kingPos, piecesAfterMove) & rookSliderBB)
                   .empty() &&
               (attacks::bishop(kingPos, piecesAfterMove) & diagSliderBB)
                   .empty();
    }

    // king movement
    if (move.pieceType == Piece::KING) {
        // castling
        if (std::optional<CastlingSide> castlingSide =
                CastlingRights::getCastlingSide(move.from, move.to,
                                                state.sideToPlay);
            castlingSide.has_value()) {
            auto [rookFrom, rookTo] = CastlingRights::getRookMovement(
                state.sideToPlay, castlingSide.value());
            Bitboard occ = getBB() ^ Bitboard::fromPos(move.from) ^
                           Bitboard::fromPos(rookTo) ^
                           Bitboard::fromPos(rookFrom) ^
                           Bitboard::fromPos(move.to);

            Bitboard thruSquares = CastlingRights::getKingThruSquares(
                state.sideToPlay, castlingSide.value());
            while (thruSquares.hasAny()) {
                Position pos = thruSquares.poplsb();
                if (attackersTo(~state.sideToPlay, pos, occ).hasAny())
                    return false;
            }
            return true;
        }

        // regular movement
        Bitboard occ =
            getBB() ^ Bitboard::fromPos(move.from) ^ Bitboard::fromPos(move.to);
        return attackersTo(~state.sideToPlay, move.to, occ).empty();
    }

    Position from = move.from;
    Position to = move.to;

    // check
    bool isNotBlocker =
        (checkInfo.blockers[static_cast<int>(state.sideToPlay)] &
         Bitboard::fromPos(from))
            .empty();
    if (checkInfo.checkers.hasAny()) {
        // moves to block check/capture checker
        Bitboard toMask = Bitboard::fromPos(to);
        Bitboard checkRayMask =
            attacks::inBetweenSquares(kingPos, checkInfo.checkers.lsb()) |
            checkInfo.checkers.lsbBB();
        if (checkInfo.checkers.count() == 1 &&
            (toMask & checkRayMask).hasAny() && isNotBlocker) {
            return true;
        }

        Bitboard occ =
            getBB() ^ Bitboard::fromPos(move.from) ^ Bitboard::fromPos(move.to);
        return attackersTo(~state.sideToPlay, kingPos, occ).empty();
    }

    // legal if it's not a blocker
    if (isNotBlocker) {
        return true;
    }

    // legal if it is along a sightline of the king
    return attacks::areAlongLine(kingPos, from, to);
}

void Game::makeNullMove() {
    this->state.sideToPlay = ~state.sideToPlay;
    zkey.flipSideToMove();
}

void Game::unmakeNullMove() {
    this->state.sideToPlay = ~state.sideToPlay;
    zkey.flipSideToMove();
}

void Game::addPiece(Position pos, Color color, Piece piece) {
    assert(pos.isValid());
    assert(!getBB().hasPos(pos));
    assert(piece != Piece::INVALID);

    Bitboard bb = Bitboard::fromPos(pos);
    board[static_cast<int>(color)][static_cast<int>(piece)] |= bb;
    colorBoard[static_cast<int>(color)] |= bb;

    pcBoard.set(pos, toPieceColorPair(piece, color));

    zkey.addPiece(piece, color, pos);
}

void Game::removePiece(Position pos, Color color, Piece piece) {
    assert(pos.isValid());
    assert(getBB(color, piece).hasPos(pos));

    Bitboard bb = ~Bitboard::fromPos(pos);
    board[static_cast<int>(color)][static_cast<int>(piece)] &= bb;
    colorBoard[static_cast<int>(color)] &= bb;

    pcBoard.set(pos, PieceColorPair::INVALID);

    zkey.removePiece(piece, color, pos);
}

void Game::movePiece(Position from, Position to, Color color, Piece piece) {
    assert(from.isValid());
    assert(to.isValid());
    assert(getBB(color, piece).hasPos(from));
    assert(!getBB().hasPos(to));

    Bitboard mask = Bitboard::fromPos(from) | Bitboard::fromPos(to);
    board[static_cast<int>(color)][static_cast<int>(piece)] ^= mask;
    colorBoard[static_cast<int>(color)] ^= mask;

    pcBoard.set(from, PieceColorPair::INVALID);
    pcBoard.set(to, toPieceColorPair(piece, color));

    zkey.movePiece(piece, color, from, to);
}

void Game::movePiece(Position from, Position to) {
    PieceColorPair pieceColorPair = getPieceColorPairAtPos(from);

    assert(pieceColorPair != PieceColorPair::INVALID);

    movePiece(from, to, getColorOf(pieceColorPair), getPieceOf(pieceColorPair));
}

void Game::updateCheckInfo() {
    Color sideToPlay = state.sideToPlay;

    Position kingPos = getBB(sideToPlay, Piece::KING).lsb();

    checkInfo.checkers = attackersTo(~sideToPlay, kingPos, getBB());
    Bitboard& pinnerBB = checkInfo.pinners[static_cast<int>(sideToPlay)];
    Bitboard& blockerBB = checkInfo.blockers[static_cast<int>(sideToPlay)];
    Bitboard& discovererBB =
        checkInfo.discoverers[static_cast<int>(~sideToPlay)];

    pinnerBB = BB_EMPTY;
    blockerBB = BB_EMPTY;
    discovererBB = BB_EMPTY;

    Bitboard queenBB = getBB(~sideToPlay, Piece::QUEEN);

    Bitboard straightPinners = (getBB(~sideToPlay, Piece::ROOK) | queenBB) &
                               attacks::rook(kingPos, BB_EMPTY);
    Bitboard diagPinners = (getBB(~sideToPlay, Piece::BISHOP) | queenBB) &
                           attacks::bishop(kingPos, BB_EMPTY);

    while (straightPinners.hasAny()) {
        Position pinnerPos = straightPinners.poplsb();
        Bitboard theseBlockers =
            attacks::inBetweenSquares(kingPos, pinnerPos) & getBB();

        if (theseBlockers.count() == 1) {
            pinnerBB |= Bitboard::fromPos(pinnerPos);

            Position blockerLsb = theseBlockers.lsb();
            if (getColorOf(pcBoard.at(blockerLsb)) == sideToPlay) {
                blockerBB |= theseBlockers;
            } else {
                discovererBB |= theseBlockers;
            }
        }
    }

    while (diagPinners.hasAny()) {
        Position pinnerPos = diagPinners.poplsb();
        Bitboard theseBlockers =
            attacks::inBetweenSquares(kingPos, pinnerPos) & getBB();

        if (theseBlockers.count() == 1) {
            pinnerBB |= Bitboard::fromPos(pinnerPos);

            Position blockerLsb = theseBlockers.lsb();
            if (getColorOf(pcBoard.at(blockerLsb)) == sideToPlay) {
                blockerBB |= theseBlockers;
            } else {
                discovererBB |= theseBlockers;
            }
        }
    }
}

Bitboard Game::attackersTo(Color attackingColor, Position pos,
                           Bitboard blockers) const {
    Bitboard queenBB = getBB(attackingColor, Piece::QUEEN);
    Bitboard rookAttackers = getBB(attackingColor, Piece::ROOK) | queenBB;
    Bitboard bishAttackers = getBB(attackingColor, Piece::BISHOP) | queenBB;

    return (rookAttackers & attacks::rook(pos, blockers)) |
           (bishAttackers & attacks::bishop(pos, blockers)) |
           (getBB(attackingColor, Piece::KNIGHT) & attacks::knight(pos)) |
           (getBB(attackingColor, Piece::KING) & attacks::king(pos)) |
           (getBB(attackingColor, Piece::PAWN) &
            attacks::pawn(pos, ~attackingColor));
}

template bool Game::canCastle<CastlingSide::KING_SIDE>() const;

template bool Game::canCastle<CastlingSide::QUEEN_SIDE>() const;

zobrist::ZobristKey Game::getZKey() const {
    return zkey;
}

bool Game::isRepetition(uint8_t maxRepetitions) const {
    return isRepetition(getZKey(), maxRepetitions);
}

bool Game::isRepetition(zobrist::ZobristKey repeatingKey,
                        uint8_t maxRepetitions) const {
    return repetition.isRepetition(repeatingKey, maxRepetitions,
                                   state.halfMoveClock + 1);
}

bool Game::hasCheckers() const {
    return checkInfo.checkers.hasAny();
}

std::ostream& operator<<(std::ostream& os, const Game& game) {
    std::ostringstream oss;

    Color sideToPlay = game.getGameState().sideToPlay;

    // :(
    for (int rank = (sideToPlay == Color::WHITE ? 7 : 0);
         (sideToPlay == Color::WHITE ? rank >= 0 : rank < 8);
         (sideToPlay == Color::WHITE ? rank-- : rank++)) {
        oss << static_cast<char>('1' + rank) << "  ";
        for (int file = 0; file < 8; file++) {
            // ReSharper disable once CppTooWideScopeInitStatement
            Bitboard bb = Bitboard::fromPos(Position(rank, file));

            if ((game.getBB(Color::WHITE, Piece::KING) & bb).hasAny())
                oss << "K ";
            else if ((game.getBB(Color::WHITE, Piece::QUEEN) & bb).hasAny())
                oss << "Q ";
            else if ((game.getBB(Color::WHITE, Piece::BISHOP) & bb).hasAny())
                oss << "B ";
            else if ((game.getBB(Color::WHITE, Piece::KNIGHT) & bb).hasAny())
                oss << "N ";
            else if ((game.getBB(Color::WHITE, Piece::ROOK) & bb).hasAny())
                oss << "R ";
            else if ((game.getBB(Color::WHITE, Piece::PAWN) & bb).hasAny())
                oss << "P ";

            else if ((game.getBB(Color::BLACK, Piece::KING) & bb).hasAny())
                oss << "k ";
            else if ((game.getBB(Color::BLACK, Piece::QUEEN) & bb).hasAny())
                oss << "q ";
            else if ((game.getBB(Color::BLACK, Piece::BISHOP) & bb).hasAny())
                oss << "b ";
            else if ((game.getBB(Color::BLACK, Piece::KNIGHT) & bb).hasAny())
                oss << "n ";
            else if ((game.getBB(Color::BLACK, Piece::ROOK) & bb).hasAny())
                oss << "r ";
            else if ((game.getBB(Color::BLACK, Piece::PAWN) & bb).hasAny())
                oss << "p ";

            else
                oss << ". ";
        }
        oss << " " << static_cast<char>('1' + rank) << "\n";
    }
    oss << "\n   ";
    for (int file = 0; file < 8; file++) {
        oss << static_cast<char>('a' + file) << " ";
    }

    oss << "\n\n";
    oss << "Hash: " << game.getZKey().getVal() << "\n";

    oss << game.getGameState() << "\n\n";

    os << oss.str();

    return os;
}
} // namespace choco
