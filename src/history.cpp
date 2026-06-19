#include "history.h"

#include <algorithm>

namespace choco {
constexpr static int16_t MAX_HISTORY = 30000;

void History::update(Color mover, const Move& move, int16_t amount) {
    int16_t& tableVal = table[static_cast<int>(mover)][move.from.getValue()]
                             [move.to.getValue()];

    int16_t clampedBonus =
        std::clamp(amount, static_cast<int16_t>(-MAX_HISTORY), MAX_HISTORY);

    tableVal += clampedBonus - tableVal * abs(clampedBonus) / MAX_HISTORY;
}

int16_t History::getBonus(Color mover, const Move& move) const {
    return table[static_cast<int>(mover)][move.from.getValue()]
                [move.to.getValue()];
}

void History::clear() {
    std::fill_n(&table[0][0][0], 2 * 64 * 64, 0);
}
} // namespace choco
