#include <catch2/catch_test_macros.hpp>
#include "solver/heat_solver.hpp"

TEST_CASE("Initialised vector of length 0 is empty"){
    std::vector<double> vect = heat::init_rod(0);
    REQUIRE(vect.empty());
}

TEST_CASE("Beginning of initialised vector is 0"){
    std::vector<double> vect = heat::init_rod(10);
    REQUIRE_FALSE(vect.empty());
    CHECK(vect.front() == 0.0);
}

TEST_CASE("End of initialised vector is 0"){
    std::vector<double> vect = heat::init_rod(10);
    REQUIRE_FALSE(vect.empty());
    CHECK(vect.back() == 0.0);
}
