<div align="center">
  <img height="240px" src="https://github.com/chococaker/castella/blob/main/castella.png?raw=true" alt="A castella">
  <h3>castella</h3>
  <p>
    A C++23 UCI-compatible chess engine.
  </p>
  <p>
    <i>&ldquo;Why in the world did you name your chess engine after a sponge cake?!&rdquo;</i>
    <br>
    <i>&ldquo;<a href="https://www.costco.com/p/-/castella-original-japanese-style-baked-sponge-cake/100770482">https://www.costco.com/p/-/castella-original-japanese-style-baked-sponge-cake/100770482</a>&rdquo;</i>
  </p>
</div>

**Castella** is a highly optimized, multithreaded chess engine written from scratch in C++. Built as the successor to my
previous engine (*Johnner*), Castella features a completely redesigned modular architecture that plays approximately
**800 Elo points stronger** than its predecessor.

This project serves as a showcase of systems-level programming, concurrent processing, and the implementation of highly
complex search and heuristic algorithms.

## What This Project Demonstrates

* **High-Performance C++:** Extensive use of bitwise operations and memory-efficient data structures to evaluate
  millions of positions per second.
* **Concurrency & Multithreading:** Implementation of Lazy Symmetric Multiprocessing (SMP) to scale performance across
  multiple CPU cores.
* **Performance Sampling:** Using profilers to accurately identify potential optimization spots with flame-graphs and
  collection logs
* **Defensive Programming:** Utilization of Catch2, perft and assertions to catch bugs early

## Core Engine Features

### Board Representation & Core Mechanics

* **Magic Bitboards:** Utilizes perfect hashing for sliding pieces (Rooks, Bishops, Queens) to achieve `O(1)` move
  generation, drastically reducing computational overhead during tree traversal.
* **Basic Time Management:** Dynamic time allocation based on the game state and remaining clock, ensuring optimal use
  of compute time per move.

### Search & Concurrency

* **Lazy SMP with Thread Voting:** Leverages multithreading by running parallel searches that share a transposition
  table, allowing the engine to scale effectively across modern multi-core processors.
* **Quiescence Search:** Extends the search at the end of the main tree to resolve noisy tactical sequences (like chains
  of captures), preventing the horizon effect.
* **Late Move Reductions (LMR):** Aggressively prunes unpromising branches to search deeper in critical lines.
* **Aspiration Windows:** Narrows the alpha-beta search window based on previous iterations to increase pruning
  efficiency.
* **Check Extensions:** Dynamically extends search depth when the king is in check to ensure accurate defensive
  calculation.

### Move Ordering (Stage-Based)

To maximize the efficiency of alpha-beta pruning, Castella orders moves to evaluate the most promising branches first:

* **TTPV (Transposition Table Principal Variation) Move:** Instantly prioritizes the best move found in previous search
  iterations.
* **MVV-LVA (Most Valuable Victim - Least Valuable Attacker):** Prioritizes winning material in capture sequences.
* **History Tables:** Tracks and prioritizes moves that frequently caused beta cutoffs in previous searches.

### Position Evaluation

* **Hand-Crafted Evaluation:** A custom, finely-tuned static evaluation function without relying on external neural
  networks.
* **Tapered Evaluation:** Smoothly interpolates piece values and positional bonuses between the opening, middlegame, and
  endgame phases.
* **Material & Piece-Square Tables (PST):** Evaluates positional advantages by incentivizing pieces to occupy
  strategically dominant squares.

## Getting Started

### Prerequisites

* A C++ compiler (GCC / Clang / MSVC)
* CMake

### Compilation

Castella uses CMake.

```bash
git clone https://github.com/chococaker/castella.git
cd castella
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build .
```

### Usage

Castella is a command-line application that uses standard chess engine protocols. You can interact with it directly via
terminal or connect it to any popular chess GUI (such as Arena, Cute Chess, or Lucas Chess) to watch it play.

```bash
./engine
```

## Note About AI

AI was not utilized in the development of this project, as it is meant primarily as a way to learn more about the world
of highly efficient simulation.

All code here reflects my **own skill** as a programmer and researcher.
