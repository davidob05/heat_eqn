#include <catch2/catch_test_macros.hpp>
#include "solver/grid.hpp"
#include <limits>

TEST_CASE("Grid initialisation fails for N<2"){
    REQUIRE_THROWS_AS(heat::Grid(0,1.0), std::invalid_argument);
}

TEST_CASE("Grid initialisation fails for 0 length"){
    REQUIRE_THROWS_AS(heat::Grid(2,0.0), std::invalid_argument);
}

TEST_CASE("Grid initialisation fails for negative length"){
    REQUIRE_THROWS_AS(heat::Grid(2,-1.0), std::invalid_argument);
}

TEST_CASE("Grid initialisation fails for infinite length"){
    REQUIRE_THROWS_AS(heat::Grid(2,std::numeric_limits<double>::infinity()), std::invalid_argument);
    REQUIRE_THROWS_AS(heat::Grid(2,-std::numeric_limits<double>::infinity()), std::invalid_argument);
}

TEST_CASE("Grid initialisation fails for NaN length"){
    REQUIRE_THROWS_AS(heat::Grid(2,std::numeric_limits<double>::quiet_NaN()), std::invalid_argument);
}


