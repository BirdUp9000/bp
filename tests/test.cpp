#define CATCH_CONFIG_MAIN
#include "catch_amalgamated.hpp"

#include "bp/bmp_reader.hpp"

TEST_CASE("Сравнение чисел") {
    int a = 42;
    int b = 42;
    REQUIRE(a == b);
}

TEST_CASE("Basic BP test") {
    BP a();
}