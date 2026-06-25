#include "search.h"

#include <cmath>
#include <utility>

#include "eval/eval.h"
#include "move_order.h"
#include "movegen.h"
#include "uci.h"

namespace choco {
SearchThread::SearchThread(int id, TT* tt, const History& history,
                           std::atomic_bool* shouldStop)
    : id(id), mainThread(false), timeMgr(nullptr), shouldStop(shouldStop),
      tt(tt), history(history), currEval(0), currDepth(0), nodesSearched(0),
      maximizingColor(Color::WHITE) {}

SearchThread::SearchThread(int id, TT* tt, const History& history,
                           std::atomic_bool* shouldStop, TimeMgr* timeMgr,
                           std::function<void()> threadVoteCallback)
    : id(id), mainThread(true), timeMgr(timeMgr), shouldStop(shouldStop),
      tt(tt), history(history), currEval(0), currDepth(0), nodesSearched(0),
      maximizingColor(Color::WHITE),
      threadVoteCallback(std::move(threadVoteCallback)) {}

void SearchThread::startSearch() {
    bestMove.reset();
    currEval = 0;
    currDepth = 1;
    nodesSearched = 0;

    std::fill_n(&pvTable[0][0], MAX_DEPTH * MAX_DEPTH, INVALID_MOVE);

    thread = std::thread([&] {
        root();
    });
    thread.detach();
}

void SearchThread::setGame(const Game& game) {
    assert(shouldStop->load());

    this->game = game;
    history.clear();
    maximizingColor = game.getGameState().sideToPlay;

    bestMove.reset();
    currEval = 0;
    currDepth = 1;
    nodesSearched = 0;
}

int SearchThread::getId() const {
    return id;
}

std::optional<Move> SearchThread::getBestMove() const {
    return bestMove;
}

int32_t SearchThread::getCurrEval() const {
    return currEval;
}

int32_t SearchThread::getVoteValue(int32_t worstScore) const {
    return bestMove.has_value() ? (currEval - worstScore + 1) * currDepth
                                : 0; // add 1 to outweigh unvoted moves
}

void SearchThread::root() {
    MoveList moves;
    generateMoves<MoveType::LEGAL>(game, moves);

    while (!shouldStop->load() && currDepth < MAX_DEPTH) {
        int32_t score = aspLoop(currDepth, currEval);

        if (shouldStop->load()) {
            break;
        }

        TTEntry ttEntry;
        tt->lookup(game.getZKey(), ttEntry, 0);
        bestMove = ttEntry.bestMove.toMove(game);

        currEval = score;

        currDepth++;

        MoveList pv;
        pv.push_back(bestMove.value());
        Move* movePtr = pvTable[0];
        while (*++movePtr != INVALID_MOVE) {
            pv.push_back(*movePtr);
        }

        if (mainThread) {
            uciInfo(currDepth, currEval, nodesSearched,
                    timeMgr->elapsed().count(), pv);

            if (timeMgr->shouldStop()) {
                shouldStop->store(true);
            }
        }
    }

    if (mainThread) {
        threadVoteCallback();
    }
}

int32_t SearchThread::aspLoop(int16_t depth, int32_t prevEval) {
    if (shouldStop->load()) return 0;

    int32_t alpha = std::numeric_limits<int32_t>::min() + 1;
    int32_t beta = std::numeric_limits<int32_t>::max() - 1;

    int32_t delta = 30;

    // allow search to stabilize first
    if (depth >= 4) {
        alpha = std::max(prevEval - delta, alpha);
        beta = std::min(prevEval + delta, beta);
    }

    while (true) {
        if (shouldStop->load()) {
            return 0;
        }

        if (mainThread && timeMgr->shouldStop()) {
            shouldStop->store(true);
            return 0;
        }

        int32_t searchScore = search(depth, alpha, beta, 0, 0);

        if (searchScore <= alpha) {
            alpha -= delta;
        } else if (searchScore >= beta) {
            beta += delta;
        } else {
            return searchScore;
        }

        delta = std::ceil(delta * 1.5);
    }
}

int32_t SearchThread::search(int16_t depth, int32_t alpha, int32_t beta,
                             int16_t numExtensions, int16_t plyFromRoot) {
    if (shouldStop->load()) return 0;
    if (mainThread && timeMgr->shouldStop()) {
        shouldStop->store(true);
        return 0;
    }

    nodesSearched++;

    if (depth == 0) return quiesce(alpha, beta);

    assert(depth > 0);

    TTEntry ttEntry = {};
    if (tt->lookup(game.getZKey(), ttEntry, 0) && ttEntry.depth >= depth) {
        if (ttEntry.flag == TTEntry::Flag::EXACT ||
            (ttEntry.flag == TTEntry::Flag::ALPHA && ttEntry.eval <= alpha) ||
            (ttEntry.flag == TTEntry::Flag::BETA && ttEntry.eval >= beta)) {
            return ttEntry.eval;
        }
    }

    int32_t best = std::numeric_limits<int32_t>::min();

    int32_t alphaOrig = alpha;

    // draw
    if (game.isRepetition(3) || game.getGameState().halfMoveClock >= 100) {
        return getDrawEval(game.getGameState().sideToPlay);
    }

    MoveOrder moveOrder =
        MoveOrder(game, ttEntry.bestMove.toMove(game), history);

    Move thisBestMove = INVALID_MOVE;

    int32_t movesSearched = 0;

    Move move = moveOrder.selectMove();

    while (move.isValid()) {
        if (!game.isLegal(move)) {
            move = moveOrder.selectMove();
            continue;
        }

        UnmakeMove unmakeMove = game.makeMove(move);

        movesSearched++;

        // Late-move reductions
        int16_t depthToSearch = depth;
        if (depth >= 3) {
            depthToSearch -= std::ceil(
                0.99 + std::log(depth) * std::log(movesSearched) / 3.14);
        } else {
            depthToSearch--;
        }

        // Search extensions
        if (numExtensions < 6 && game.hasCheckers()) {
            numExtensions++;
            depthToSearch++;
        }

        int32_t score =
            -search(depthToSearch, -beta, -alpha, numExtensions, plyFromRoot + 1);

        if (score >= eval::MATE_EVAL_THRESHOLD) {
            --score;
        } else if (score <= -eval::MATE_EVAL_THRESHOLD) {
            ++score;
        }

        if (score > best) {
            best = score;
            thisBestMove = move;
            if (score > alpha) alpha = score;

            // copy pv
            pvTable[plyFromRoot][0] = move;
            Move* m = pvTable[plyFromRoot + 1];
            int i;
            // if there's a PV that's at MAX_DEPTH, womp womp
            for (i = 0; i < MAX_DEPTH - 3; i++) {
                pvTable[plyFromRoot][i + 1] = *m;
                m++;
                if (!m->isValid()) {
                    break;
                }
            }
            // add INVALID_MOVE after as a marker to stop reading PV
            pvTable[plyFromRoot][i + 2] = INVALID_MOVE;
        }

        game.unmakeMove(unmakeMove);

        // Fail high beta cutoff
        if (score >= beta) {
            tt->store(game.getZKey(), best, depth, move, TTEntry::Flag::BETA);

            if (game.getPieceColorPairAtPos(move.to) ==
                PieceColorPair::INVALID) {
                history.update(game.getGameState().sideToPlay, move,
                               depth * depth);
            }

            return best;
        }

        move = moveOrder.selectMove();
    }

    // game over check
    // mates
    if (thisBestMove == INVALID_MOVE) {
        best = game.hasCheckers() ? -eval::MATE_EVAL
                                  : getDrawEval(game.getGameState().sideToPlay);
    }

    TTEntry::Flag flag;
    if (best >= beta) {
        flag = TTEntry::Flag::BETA;
    } else if (best <= alphaOrig) {
        flag = TTEntry::Flag::ALPHA;
    } else {
        flag = TTEntry::Flag::EXACT;
    }

    tt->store(game.getZKey(), best, depth, thisBestMove, flag);

    return best;
}

int32_t SearchThread::quiesce(int32_t alpha, int32_t beta) {
    nodesSearched++;

    int32_t best = eval::staticEval(game);

    // Stand Pat
    if (best >= beta) return best;
    if (best > alpha) alpha = best;

    MoveOrder moveOrder = MoveOrder(game);

    Move move = moveOrder.selectMove();

    while (move.isValid()) {
        if (!game.isLegal(move)) {
            move = moveOrder.selectMove();
            continue;
        }

        UnmakeMove unmakeMove = game.makeMove(move);

        int32_t score = -quiesce(-beta, -alpha);
        game.unmakeMove(unmakeMove);

        // Fail hard beta cutoff
        if (score > best) best = score;
        if (score > alpha) alpha = score;
        if (score >= beta) return best;

        move = moveOrder.selectMove();
    }

    return best;
}

int32_t SearchThread::getDrawEval(Color color) const {
    return color == maximizingColor ? eval::DRAW_EVAL : -eval::DRAW_EVAL;
}

Search::Search() : bestMove(std::nullopt), currEval(0), nodes(0) {
    setThreadCount(4);
}

void Search::setGame(const Game& newGame, bool clear) {
    timeMgr.forceStopSearch();
    game = newGame;
    bestMove = std::nullopt;

    if (clear) {
        tt.clear();
    }
}

void Search::startSearch(const SearchBounds& searchBounds) {
    timeMgr.startSearch(searchBounds);
    shouldStop.store(true);

    for (SearchThread* searchThread : searchThreads) {
        searchThread->setGame(game);
    }

    shouldStop.store(false);

    for (SearchThread* searchThread : searchThreads) {
        searchThread->startSearch();
    }
}

void Search::endSearch() {
    timeMgr.forceStopSearch();
    shouldStop.store(true);
}

void Search::setThreadCount(int count) {
    assert(timeMgr.shouldStop());
    assert(count >= 1);

    for (SearchThread* searchThread : searchThreads) {
        delete searchThread;
    }
    searchThreads.clear();

    // main thread
    searchThreads.push_back(
        new SearchThread(0, &tt, History(), &shouldStop, &timeMgr, [&] {
            reportBestMove();
        }));

    for (int i = 1; i < count; i++) {
        searchThreads.push_back(
            new SearchThread(i, &tt, History(), &shouldStop));
    }
}

bool Search::isSearching() {
    return !timeMgr.shouldStop();
}

std::optional<Move> Search::getBestMove() const {
    return bestMove;
}

int32_t Search::getCurrEval() const {
    return currEval;
}

TimeMgr& Search::getTimeMgr() {
    return timeMgr;
}

Search::~Search() {
    for (SearchThread* searchThread : searchThreads) {
        delete searchThread;
    }
}

void Search::reportBestMove() const {
    MoveList moveList;
    generateMoves<MoveType::LEGAL>(game, moveList);

    std::vector<int> votes = std::vector<int>();
    votes.resize(moveList.size(), 0);

    // find lowest value
    int32_t lowestEval = std::numeric_limits<int32_t>::max();

    for (SearchThread* searchThread : searchThreads) {
        int32_t threadEval = searchThread->getCurrEval();
        if (threadEval < lowestEval) {
            lowestEval = threadEval;
        }
    }

    // do the voting thang
    for (SearchThread* searchThread : searchThreads) {
        Move move = searchThread->getBestMove().value_or(INVALID_MOVE);

        // find index
        int idx = -1;
        for (int i = 0; i < moveList.size(); i++) {
            if (move == moveList[i]) {
                idx = i;
                break;
            }
        }

        if (idx == -1) continue; // idk how this would happen lowkey

        votes[idx] += searchThread->getVoteValue(lowestEval);
    }

    int highestIdx = 0;
    int32_t highestVoteVal = 0;

    for (int i = 0; i < votes.size(); i++) {
        if (votes[i] >= highestVoteVal) {
            highestVoteVal = votes[i];
            highestIdx = i;
        }
    }

    uciBestMove(moveList[highestIdx]);
}
} // namespace choco
