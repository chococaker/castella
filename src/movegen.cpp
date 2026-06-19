#include "movegen.h"

#include "attacks.h"

namespace choco {
template<MoveType moveType>
void generateKingMoves(const Game& game, MoveList& moves);

template<MoveType moveType>
void generateQueenMoves(const Game& game, MoveList& moves);

template<MoveType moveType>
void generateBishopMoves(const Game& game, MoveList& moves);

template<MoveType moveType>
void generateKnightMoves(const Game& game, MoveList& moves);

template<MoveType moveType>
void generateRookMoves(const Game& game, MoveList& moves);

template<MoveType moveType>
void generatePawnMoves(const Game& game, MoveList& moves);

template<MoveType moveType>
void generateMoves(const Game& game, MoveList& moves) {
    generateKnightMoves<moveType>(game, moves);
    generateQueenMoves<moveType>(game, moves);
    generateBishopMoves<moveType>(game, moves);
    generateRookMoves<moveType>(game, moves);
    generatePawnMoves<moveType>(game, moves);
    generateKingMoves<moveType>(game, moves);
}

template void generateMoves<MoveType::NOISY>(const Game& game, MoveList& moves);

template void generateMoves<MoveType::QUIET>(const Game& game, MoveList& moves);

template void generateMoves<MoveType::LEGAL>(const Game& game, MoveList& moves);

template<MoveType moveType>
constexpr Bitboard getEffectiveOccupancies(const Game& game) {
    Bitboard occupied = game.getBB(game.getGameState().sideToPlay);

    if constexpr (moveType == MoveType::QUIET) {
        occupied |= game.getBB(~game.getGameState().sideToPlay);
    }

    return occupied;
}

template<Piece piece, bool checkLegal>
constexpr void writeMoves(const Game& game, Position from, Bitboard to,
                          MoveList& moveList) {
    while (to.hasAny()) {
        Move move = Move(piece, from, to.poplsb());
        if constexpr (checkLegal) {
            if (!game.isLegal(move)) continue;
        }
        moveList.push_back(move);
    }
}

/**
 * @tparam promotion Whether the pawns are promoting
 * @param pawnsTo The resulting bitboard from the movement
 * @param moveList
 * @param moveDir The direction the pawns moved in to get to their current
 * position
 */
template<bool promotion, bool checkLegal>
constexpr void writePawnMoves(const Game& game, Bitboard pawnsTo,
                              MoveList& moveList, Direction moveDir) {
    while (pawnsTo.hasAny()) {
        Position to = pawnsTo.poplsb();
        Position from = to - moveDir;

        if constexpr (promotion) {
            Move queenPromot = Move(Piece::PAWN, from, to, Piece::QUEEN);
            if constexpr (checkLegal) {
                if (!game.isLegal(queenPromot)) continue;
            }

            moveList.push_back(queenPromot);
            moveList.push_back({Piece::PAWN, from, to, Piece::BISHOP});
            moveList.push_back({Piece::PAWN, from, to, Piece::KNIGHT});
            moveList.push_back({Piece::PAWN, from, to, Piece::ROOK});
        } else {
            Move move = Move(Piece::PAWN, from, to);
            if constexpr (checkLegal) {
                if (!game.isLegal(move)) continue;
            }

            moveList.push_back(move);
        }
    }
}

template<MoveType moveType>
void generateKingMoves(const Game& game, MoveList& moves) {
    Bitboard occupancies = getEffectiveOccupancies<moveType>(game);
    Bitboard pieces = game.getBB(game.getGameState().sideToPlay, Piece::KING);

    while (pieces.hasAny()) {
        Position piecePos = pieces.poplsb();
        Bitboard placesToGo = attacks::king(piecePos);
        placesToGo &= ~occupancies;

        // castling
        if (game.canCastle<CastlingSide::KING_SIDE>()) {
            Position castlePos = CastlingRights::getCastlingPosition(
                game.getGameState().sideToPlay, CastlingSide::KING_SIDE);
            placesToGo |= Bitboard::fromPos(castlePos);
        }
        if (game.canCastle<CastlingSide::QUEEN_SIDE>()) {
            Position castlePos = CastlingRights::getCastlingPosition(
                game.getGameState().sideToPlay, CastlingSide::QUEEN_SIDE);
            placesToGo |= Bitboard::fromPos(castlePos);
        }

        if constexpr (moveType == MoveType::NOISY) {
            placesToGo &= game.getBB(~game.getGameState().sideToPlay);
        }

        writeMoves<Piece::KING, moveType == MoveType::LEGAL>(game, piecePos,
                                                             placesToGo, moves);
    }
}

template<MoveType moveType>
void generateQueenMoves(const Game& game, MoveList& moves) {
    Bitboard occupancies = getEffectiveOccupancies<moveType>(game);
    Bitboard pieces = game.getBB(game.getGameState().sideToPlay, Piece::QUEEN);

    while (pieces.hasAny()) {
        Position piecePos = pieces.poplsb();
        Bitboard placesToGo = attacks::queen(piecePos, game.getBB());
        placesToGo &= ~occupancies;

        if constexpr (moveType == MoveType::NOISY) {
            placesToGo &= game.getBB(~game.getGameState().sideToPlay);
        }

        writeMoves<Piece::QUEEN, moveType == MoveType::LEGAL>(
            game, piecePos, placesToGo, moves);
    }
}

template<MoveType moveType>
void generateBishopMoves(const Game& game, MoveList& moves) {
    Bitboard occupancies = getEffectiveOccupancies<moveType>(game);
    Bitboard pieces = game.getBB(game.getGameState().sideToPlay, Piece::BISHOP);

    while (pieces.hasAny()) {
        Position piecePos = pieces.poplsb();
        Bitboard placesToGo = attacks::bishop(piecePos, game.getBB());
        placesToGo &= ~occupancies;

        if constexpr (moveType == MoveType::NOISY) {
            placesToGo &= game.getBB(~game.getGameState().sideToPlay);
        }

        writeMoves<Piece::BISHOP, moveType == MoveType::LEGAL>(
            game, piecePos, placesToGo, moves);
    }
}

template<MoveType moveType>
void generateKnightMoves(const Game& game, MoveList& moves) {
    Bitboard occupancies = getEffectiveOccupancies<moveType>(game);
    Bitboard pieces = game.getBB(game.getGameState().sideToPlay, Piece::KNIGHT);

    while (pieces.hasAny()) {
        Position piecePos = pieces.poplsb();
        Bitboard placesToGo = attacks::knight(piecePos);
        placesToGo &= ~occupancies;

        if constexpr (moveType == MoveType::NOISY) {
            placesToGo &= game.getBB(~game.getGameState().sideToPlay);
        }

        writeMoves<Piece::KNIGHT, moveType == MoveType::LEGAL>(
            game, piecePos, placesToGo, moves);
    }
}

template<MoveType moveType>
void generateRookMoves(const Game& game, MoveList& moves) {
    Bitboard occupancies = getEffectiveOccupancies<moveType>(game);
    Bitboard pieces = game.getBB(game.getGameState().sideToPlay, Piece::ROOK);

    while (pieces.hasAny()) {
        Position piecePos = pieces.poplsb();
        Bitboard placesToGo = attacks::rook(piecePos, game.getBB());
        placesToGo &= ~occupancies;

        if constexpr (moveType == MoveType::NOISY) {
            placesToGo &= game.getBB(~game.getGameState().sideToPlay);
        }

        writeMoves<Piece::ROOK, moveType == MoveType::LEGAL>(game, piecePos,
                                                             placesToGo, moves);
    }
}

template<MoveType moveType>
void generatePawnMoves(const Game& game, MoveList& moves) {
    const Bitboard pawnBB =
        game.getBB(game.getGameState().sideToPlay, Piece::PAWN);
    Direction forward = game.getGameState().sideToPlay == Color::WHITE
                            ? Direction::UP
                            : Direction::DOWN;
    Bitboard promotingRank =
        game.getGameState().sideToPlay == Color::WHITE ? BB_RANK_7 : BB_RANK_2;

    Color sideToPlay = game.getGameState().sideToPlay;

    Bitboard unoccupied = ~game.getBB();

    if constexpr (moveType == MoveType::QUIET || moveType == MoveType::LEGAL) {
        /** Non-promoting pawn pushes **/
        // Move pawnBB (excluding the promoting pawns) forward, and exclude ones
        // that would have moved onto an occupied position
        Bitboard singlePushBB =
            (pawnBB & ~promotingRank).up(sideToPlay) & unoccupied;
        writePawnMoves<false, moveType == MoveType::LEGAL>(game, singlePushBB,
                                                           moves, forward);

        /** Double pawn pushes **/
        // Basically same procedure as single-push
        Bitboard doublePushRankBB =
            sideToPlay == Color::WHITE ? BB_RANK_3 : BB_RANK_6;
        Bitboard doublePushBB =
            (singlePushBB & doublePushRankBB).up(sideToPlay) & unoccupied;
        writePawnMoves<false, moveType == MoveType::LEGAL>(
            game, doublePushBB, moves, forward + forward);
    }

    if constexpr (moveType == MoveType::NOISY || moveType == MoveType::LEGAL) {
        Bitboard enemyBB = game.getBB(~sideToPlay);

        // Add en passant position if valid
        if (game.getGameState().enpassantPos.isValid()) {
            enemyBB |= Bitboard::fromPos(game.getGameState().enpassantPos);
        }

        /** Non-promoting captures **/
        // Left captures
        Bitboard captureLeftBB =
            (pawnBB & ~BB_FILE_A & ~promotingRank).up(sideToPlay).left() &
            enemyBB;
        // Right captures
        Bitboard captureRightBB =
            (pawnBB & ~BB_FILE_H & ~promotingRank).up(sideToPlay).right() &
            enemyBB;
        writePawnMoves<false, moveType == MoveType::LEGAL>(
            game, captureLeftBB, moves, forward + Direction::LEFT);
        writePawnMoves<false, moveType == MoveType::LEGAL>(
            game, captureRightBB, moves, forward + Direction::RIGHT);

        /** Promoting captures **/
        // Left captures (reuse variable)
        captureLeftBB =
            (pawnBB & ~BB_FILE_A & promotingRank).up(sideToPlay).left() &
            enemyBB;
        // Right captures (reuse variable)
        captureRightBB =
            (pawnBB & ~BB_FILE_H & promotingRank).up(sideToPlay).right() &
            enemyBB;
        writePawnMoves<true, moveType == MoveType::LEGAL>(
            game, captureLeftBB, moves, forward + Direction::LEFT);
        writePawnMoves<true, moveType == MoveType::LEGAL>(
            game, captureRightBB, moves, forward + Direction::RIGHT);

        /** Promoting pushes **/
        Bitboard promotedPushesBB =
            (pawnBB & promotingRank).up(sideToPlay) & unoccupied;
        writePawnMoves<true, moveType == MoveType::LEGAL>(
            game, promotedPushesBB, moves, forward);
    }
}

bool isPseudoLegal(const Move& move, const Game& game) {
    if (!move.isValid()) {
        return false;
    }

    PieceColorPair pcpFrom = game.getPieceColorPairAtPos(move.from);

    // Piece exists at origin check
    if (pcpFrom == PieceColorPair::INVALID) {
        return false;
    }

    // NOTE: now that we've checked that the piece is correct, we can assume
    // that the move would show up in an attack table with no occupancies.

    Color sideToPlay = game.getGameState().sideToPlay;

    // Color check
    if (getColorOf(pcpFrom) != sideToPlay) {
        return false;
    }

    // Piece check
    if (getPieceOf(pcpFrom) != move.pieceType) {
        return false;
    }

    PieceColorPair pcpTo = game.getPieceColorPairAtPos(move.to);

    // Destination occupation check
    if (pcpTo != PieceColorPair::INVALID && getColorOf(pcpTo) == sideToPlay) {
        return false;
    }

    Bitboard fromBB = Bitboard::fromPos(move.from);
    Bitboard toBB = Bitboard::fromPos(move.to);

    // Pawn check
    if (move.pieceType == Piece::PAWN) {
        if (fromBB.up(sideToPlay) == toBB) { // single push
            if ((game.getBB(~sideToPlay) & toBB).hasAny())
                return false; // you are pushing into an enemy piece
        } else if (fromBB.up(sideToPlay).up(sideToPlay) ==
                   toBB) { // double pushing
            Bitboard oneInFront = fromBB.up(game.getGameState().sideToPlay);
            if ((game.getBB(~sideToPlay) & (oneInFront | toBB)).hasAny())
                return false;
        } else { // must be a capture/ep
            if (pcpTo == PieceColorPair::INVALID &&
                game.getGameState().enpassantPos != move.to)
                return false;
        }
    } else { // All the other stuff
        Bitboard effectiveOccupancies =
            getEffectiveOccupancies<MoveType::LEGAL>(game);
        Bitboard attacks =
            attacks::attacks(move.pieceType, move.from, effectiveOccupancies);

        if ((attacks & toBB).empty()) {
            return false;
        }
    }

    return true;
}
} // namespace choco
