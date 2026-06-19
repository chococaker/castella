add_executable(tests
        attacks.test.cpp
        game.test.cpp
        main.test.cpp
        move_order.test.cpp
        movegen.test.cpp
        types.test.cpp
        zobrist.test.cpp
)

target_include_directories(tests PRIVATE ${CMAKE_SOURCE_DIR}/src)

target_link_libraries(tests PRIVATE doctest::doctest)
target_link_libraries(tests PRIVATE core)

add_test(NAME engine_tests COMMAND tests)
