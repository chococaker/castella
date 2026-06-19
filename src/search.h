#pragma once

#include "game.h"
#include "history.h"
#include "time_mgr.h"
#include "tt.h"

#include <functional>
#include <thread>

namespace choco {
class SearchThread {
  public:
    SearchThread(int id, TT* tt, const History& history,
                 std::atomic_bool* shouldStop);
    /**
     * Makes this thread the main thread
     * @param id
     * @param tt
     * @param history
     * @param shouldStop
     * @param timeMgr
     * @param threadVoteCallback What to call when the search is over
     */
    SearchThread(int id, TT* tt, const History& history,
                 std::atomic_bool* shouldStop, TimeMgr* timeMgr,
                 std::function<void()> threadVoteCallback);

    void startSearch();

    void setGame(const Game& game);

    int getId() const;

    std::optional<Move> getBestMove() const;
    int32_t getCurrEval() const;

    int32_t getVoteValue(int32_t worstScore) const;

  private:
    void root();

    int32_t aspLoop(int16_t depth, int32_t prevEval);
    int32_t search(int16_t depth, int32_t alpha, int32_t beta, int16_t numExtensions);
    int32_t quiesce(int32_t alpha, int32_t beta);

    int32_t getDrawEval(Color color) const;

    int id;
    bool mainThread;

    TimeMgr* timeMgr;
    std::atomic_bool* shouldStop;

    TT* tt;

    std::thread thread;

    History history;

    std::optional<Move> bestMove;
    int32_t currEval;

    int16_t currDepth;

    uint64_t nodesSearched;

    Game game;

    Color maximizingColor;

    std::function<void()> threadVoteCallback;
};

class Search {
  public:
    Search();
    explicit Search(const SearchThread& other) = delete;

    void setGame(const Game& newGame, bool clear);

    void startSearch(const SearchBounds& searchBounds);

    void endSearch();

    void setThreadCount(int count);

    bool isSearching();

    std::optional<Move> getBestMove() const;

    int32_t getCurrEval() const;

    TimeMgr& getTimeMgr();

    Search& operator=(const Search& other) = delete;

    ~Search();

  private:
    Game game;

    std::optional<Move> bestMove;
    int32_t currEval;

    TT tt;

    TimeMgr timeMgr;

    uint32_t nodes;

    std::vector<SearchThread*> searchThreads;

    std::atomic_bool shouldStop;

    void reportBestMove() const;
};
} // namespace choco
