set(PROJECT_SOURCES
        attacks.cpp
        game.cpp
        history.cpp
        move_order.cpp
        movegen.cpp
        perft.cpp
        search.cpp
        time_mgr.cpp
        types.cpp
        uci.cpp
        zobrist.cpp

        eval/eval.cpp
)

add_library(core ${PROJECT_SOURCES})

target_include_directories(core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

set(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=native -DNDEBUG")

set(CMAKE_CXX_FLAGS_DEBUG "-g")

add_executable(engine main.cpp)
target_link_libraries(engine PRIVATE core)
