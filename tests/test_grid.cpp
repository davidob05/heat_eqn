#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include "core/grid.hpp"
#include <limits>
#include <stdexcept>
#include <catch2/generators/catch_generators_range.hpp>



TEST_CASE("Grid initialisation fails for less than 1 interval"){
    auto length = GENERATE(as<double>{},0.7,1.0,100.58,0.1);
    INFO("built from length = " << length);
    CHECK_THROWS_AS(heat::Grid(0,length), std::invalid_argument);
}

TEST_CASE("Grid initialisation fails for 0 length"){
    auto n_intervals = GENERATE(as<std::size_t>{},12,20,1000,1,49);
    INFO("built from intervals = " << n_intervals);
    CHECK_THROWS_AS(heat::Grid(n_intervals,0.0), std::invalid_argument);
}

TEST_CASE("Grid initialisation fails for negative length"){
    auto n_intervals = GENERATE(as<std::size_t>{},12,20,1000,1,49);
    INFO("built from intervals = " << n_intervals);
    CHECK_THROWS_AS(heat::Grid(n_intervals,-1.0), std::invalid_argument);
}

TEST_CASE("Grid initialisation fails for infinite length"){
    auto n_intervals = GENERATE(as<std::size_t>{},12,20,1000,1,49);
    INFO("built from intervals = " << n_intervals);
    CHECK_THROWS_AS(heat::Grid(n_intervals,std::numeric_limits<double>::infinity()), std::invalid_argument);
    CHECK_THROWS_AS(heat::Grid(n_intervals,-std::numeric_limits<double>::infinity()), std::invalid_argument);
}

TEST_CASE("Grid initialisation fails for NaN length"){
    auto n_intervals = GENERATE(as<std::size_t>{},12,20,1000,1,49);
    INFO("built from intervals = " << n_intervals);
    CHECK_THROWS_AS(heat::Grid(n_intervals,std::numeric_limits<double>::quiet_NaN()), std::invalid_argument);
}

TEST_CASE("Grid has exactly one more item than given intervals"){
    auto n_intervals = GENERATE(as<std::size_t>{},12,20,1000,1,49);
    auto length = GENERATE(as<double>{},0.7,1.0,100.58,0.1);
    INFO("built from intervals = " << n_intervals << ", and length = " << length);
    CHECK(heat::Grid(n_intervals,length).size() == n_intervals+1);
}

TEST_CASE("First position of Grid is exactly 0"){
    auto n_intervals = GENERATE(as<std::size_t>{},12,20,1000,1,49);
    auto length = GENERATE(as<double>{},0.7,1.0,100.58,0.1);
    INFO("built from intervals = " << n_intervals << ", and length = " << length);
    CHECK(heat::Grid(n_intervals,length).x()[0] == 0);
}

TEST_CASE("Last position of Grid is exactly L"){
    auto n_intervals = GENERATE(range<std::size_t>(1, 200));
    auto length = GENERATE(as<double>{},0.7,1.0,100.58,0.1);
    INFO("built from intervals = " << n_intervals << ", and length = " << length);
    CHECK(heat::Grid(n_intervals,length).x().back() == length);
}

TEST_CASE("Gap between each position is dx"){
    auto n_intervals = GENERATE(as<std::size_t>{},12,20,1000,1,49);
    auto length = GENERATE(as<double>{},0.7,1.0,100.58,0.1);
    heat::Grid grid(n_intervals,length);
    INFO("built from intervals = " << n_intervals << ", and length = " << length); 
    for(std::size_t i = 1; i < grid.size() ;i++){
        CHECK_THAT(grid.x()[i] - grid.x()[i-1], Catch::Matchers::WithinRel(grid.dx(),std::numeric_limits<double>::epsilon()*4*n_intervals));
    }
}

TEST_CASE("dx is L/N"){
    auto n_intervals = GENERATE(as<std::size_t>{},12,20,1000,1,49);
    auto length = GENERATE(as<double>{},0.7,1.0,100.58,0.1);
    INFO("built from intervals = " << n_intervals << ", and length = " << length); 
    CHECK(heat::Grid(n_intervals,length).dx()==length/n_intervals);
}
