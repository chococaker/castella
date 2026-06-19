#pragma once

#include "game.h"
#include "history.h"
#include "tt.h"

namespace choco {
enum struct SelectionStage {
    TT_MOVE,
    GENERATE_NOISY,
    WINNING_MVVLVA,
    GENERATE_QUIET,
    HISTORY_AND_LOSING_MVVLVA,

    QUIESCE
};

struct ScoredMove {
    int32_t score;
    Move move;
};

class MoveOrder {
  public:
    /**
     * For quiescence
     * @param game
     */
    explicit MoveOrder(const Game& game);

    /**
     * For normal
     *
     * @param game
     * @param pvMove
     * @param history
     */
    MoveOrder(const Game& game, const Move& pvMove, const History& history);

    MoveOrder(const MoveOrder& other) = default;

    const Move& selectMove();

  private:
    SelectionStage selectionStage;

    const Game& game;
    ScoredMove scoredMoves[218];
    uint8_t size;

    Move pvMove;
    const History* history;

    uint8_t currIndex;

    const ScoredMove& selectHighest();
    void nextStage();
};
} // namespace choco
