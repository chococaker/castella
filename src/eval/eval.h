#pragma once

#include "game.h"

namespace choco::eval {
static constexpr int32_t MATE_EVAL = 3200000;
static constexpr int32_t MATE_EVAL_THRESHOLD = 3000000;

static constexpr int32_t DRAW_EVAL = -25;

int32_t getMaterialValue(Piece piece);

int32_t staticEval(const Game& game);
} // namespace choco::eval
