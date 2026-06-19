#pragma once

#include <atomic>
#include <chrono>

namespace choco {
struct SearchBounds {
    std::chrono::milliseconds maxTime;
    int16_t maxDepth;
    std::chrono::milliseconds clockTime;
    std::chrono::milliseconds clockInc;
};

class TimeMgr {
  public:
    TimeMgr();

    void startSearch(const SearchBounds& sb);

    void forceStopSearch();

    std::chrono::milliseconds elapsed() const;

    bool shouldStop();

  private:
    SearchBounds searchBounds;
    std::chrono::steady_clock::time_point startTime;
    std::chrono::milliseconds searchTime;
    std::atomic_bool stopped;
};
} // namespace choco
