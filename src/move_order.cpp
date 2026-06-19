#include "move_order.h"

#include "movegen.h"

namespace choco {
constexpr static int32_t WINNING_MVVLVA = 100000;
constexpr static int32_t LOSING_MVVLVA = -100000;

namespace {
int32_t mvvLva(const Game& game, Move move) {
    Piece capturedPiece = getPieceOf(game.getPieceColorPairAtPos(move.to));
    if (capturedPiece == Piece::INVALID) return 0;

    int32_t capturedVal = eval::getMaterialValue(capturedPiece);

    int32_t takerVal = eval::getMaterialValue(move.pieceType);

    int32_t res = capturedVal - takerVal;

    res += res >= 0 ? WINNING_MVVLVA : LOSING_MVVLVA;

    return res;
}
} // namespace

MoveOrder::MoveOrder(const Game& game)
    : selectionStage(SelectionStage::QUIESCE), game(game), scoredMoves(),
      pvMove(INVALID_MOVE), history(nullptr), currIndex(0) {
    // generate moves from the get-go

    MoveList moveList;
    generateMoves<MoveType::NOISY>(game, moveList);

    for (int i = 0; i < moveList.size(); i++) {
        scoredMoves[i] = {mvvLva(game, moveList[i]), moveList[i]};
    }

    size = moveList.size();
}

MoveOrder::MoveOrder(const Game& game, const Move& pvMove,
                     const History& history)
    : game(game), scoredMoves(), size(0), history(&history), currIndex(0) {
    if (isPseudoLegal(pvMove, game)) {
        selectionStage = SelectionStage::TT_MOVE;
        this->pvMove = pvMove;
    } else {
        selectionStage = SelectionStage::GENERATE_NOISY;
        this->pvMove = INVALID_MOVE;
    }
}

const ScoredMove& MoveOrder::selectHighest() {
    int bestIdx = -1;
    int32_t bestScore = std::numeric_limits<int32_t>::min();

    for (int i = currIndex; i < size; i++) {
        if (scoredMoves[i].score > bestScore && scoredMoves[i].move != pvMove) {
            bestScore = scoredMoves[i].score;
            bestIdx = i;
        }
    }

    if (currIndex == size || bestIdx == -1) {
        static constexpr ScoredMove SENTINEL = {0, INVALID_MOVE};
        return SENTINEL;
    } else {
        std::swap(scoredMoves[currIndex], scoredMoves[bestIdx]);
        return scoredMoves[currIndex++];
    }
}

const Move& MoveOrder::selectMove() {
    switch (selectionStage) {
        /*** LEGAL MOVES **/
    case SelectionStage::TT_MOVE: {
        nextStage();
        return pvMove;
    }

    case SelectionStage::GENERATE_NOISY: {
        MoveList moveList;
        generateMoves<MoveType::NOISY>(game, moveList);

        for (int i = 0; i < moveList.size(); i++) {
            scoredMoves[i] = {mvvLva(game, moveList[i]), moveList[i]};
        }

        size = moveList.size();

        nextStage();
        // fallthrough
    }

    case SelectionStage::WINNING_MVVLVA: {
        int32_t highestScoreSoFar = std::numeric_limits<int32_t>::min();
        int highestIndexSoFar = -1;

        for (int i = currIndex; i < size; i++) {
            if (highestScoreSoFar < scoredMoves[i].score &&
                scoredMoves[i].score > 0 && scoredMoves[i].move != pvMove) {
                highestScoreSoFar = scoredMoves[i].score;
                highestIndexSoFar = i;
            }
        }

        if (currIndex == size || highestIndexSoFar == -1) {
            nextStage();
            // fallthrough
        } else {
            std::swap(scoredMoves[currIndex], scoredMoves[highestIndexSoFar]);
            return scoredMoves[currIndex++].move;
        }
    }

    case SelectionStage::GENERATE_QUIET: {
        assert(history != nullptr);

        MoveList moveList;
        generateMoves<MoveType::QUIET>(game, moveList);

        for (int i = 0; i < moveList.size(); i++) {
            const Move& move = moveList[i];
            scoredMoves[i + size] = {
                history->getBonus(game.getGameState().sideToPlay, move), move};
        }

        size += moveList.size();

        assert(size < 218);

        nextStage();
        // fallthrough
    }

    case SelectionStage::HISTORY_AND_LOSING_MVVLVA: {
        return selectHighest().move;
    }

        /*** QUIESCENCE MOVES **/
    case SelectionStage::QUIESCE: {
        int32_t highestScoreSoFar = std::numeric_limits<int32_t>::min();
        int highestIndexSoFar = -1;

        for (int i = currIndex; i < size; i++) {
            if (highestScoreSoFar < scoredMoves[i].score) {
                highestScoreSoFar = scoredMoves[i].score;
                highestIndexSoFar = i;
            }
        }

        if (currIndex == size || highestIndexSoFar == -1) {
            return INVALID_MOVE;
        } else {
            std::swap(scoredMoves[currIndex], scoredMoves[highestIndexSoFar]);
            return scoredMoves[currIndex++].move;
        }
    }
    }

    assert(0);
}

void MoveOrder::nextStage() {
    selectionStage =
        static_cast<SelectionStage>(static_cast<int>(selectionStage) + 1);
}
} // namespace choco
