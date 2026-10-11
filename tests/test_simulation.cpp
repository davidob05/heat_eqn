#include <catch2/catch_test_macros.hpp>
#include "driver/simulation.hpp"
#include <sstream>
#include <stdexcept>
#include <ios>

TEST_CASE("run_simulation throws if the output stream has failed"){
    heat::SimulationParams params;
    std::ostringstream stream;
    stream.setstate(std::ios::badbit);
    CHECK_THROWS_AS(heat::run_simulation(params,stream),std::runtime_error);
}

TEST_CASE("run_simulation does not throw on a healthy stream"){
    heat::SimulationParams params;
    std::ostringstream stream;
    CHECK_NOTHROW(heat::run_simulation(params,stream));
    CHECK(!stream.str().empty());
}
