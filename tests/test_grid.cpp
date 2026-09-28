#include <catch2/catch_test_macros.hpp>
#include "solver/grid.hpp"

TEST_CASE("Grid initialisation fails for N<2"){
    REQUIRE_THROWS_AS(heat::Grid(1,1.0), std::invalid_argument);
    REQUIRE_THROWS_AS(heat::Grid(0,1.0), std::invalid_argument);
}


