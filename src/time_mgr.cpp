#include "time_mgr.h"

namespace choco {
constexpr static std::chrono::milliseconds ZERO_MS =
    std::chrono::milliseconds::zero();

TimeMgr::TimeMgr() : searchBounds(), searchTime(ZERO_MS) {
    stopped.store(true, std::memory_order_relaxed);
}

void TimeMgr::startSearch(const SearchBounds& sb) {
    searchBounds = sb;
    stopped.store(false, std::memory_order_relaxed);
    startTime = std::chrono::steady_clock::now();

    // calculate allowed search time
    if (searchBounds.maxTime != ZERO_MS) {
        searchTime = searchBounds.maxTime;
    } else if (searchBounds.clockTime != ZERO_MS) {
        searchTime =
            (searchBounds.clockTime / 20) + (searchBounds.clockInc / 2);
    } else {
        searchTime = ZERO_MS;
    }
}

void TimeMgr::forceStopSearch() {
    stopped.store(true, std::memory_order_relaxed);
}

std::chrono::milliseconds TimeMgr::elapsed() const {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - startTime);
}

bool TimeMgr::shouldStop() {
    if (stopped.load(std::memory_order_relaxed)) {
        return true;
    }

    if (searchTime != ZERO_MS && elapsed() >= searchTime) {
        stopped.store(true, std::memory_order_relaxed);
        return true;
    }

    return false;
}
} // namespace choco
