cmake_minimum_required(VERSION 3.15...4.3.1)

project(castella
        VERSION 1.0
        LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_EXTENSIONS OFF)

add_subdirectory(src)

# TESTING
enable_testing()

include(FetchContent)
FetchContent_Declare(
        doctest
        GIT_REPOSITORY https://github.com/doctest/doctest.git
        GIT_TAG        v2.5.0
)
FetchContent_MakeAvailable(doctest)

add_subdirectory(tests)
