#pragma once
#include <cstdint>

#include "types.h"

namespace choco {
class History {
  public:
    History() = default;
    History(const History& other) = default;

    void update(Color mover, const Move& move, int16_t amount);

    int16_t getBonus(Color mover, const Move& move) const;

    void clear();

  private:
    int16_t table[2][64][64] {}; // side to move, from, to
};
} // namespace choco
