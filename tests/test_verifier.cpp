#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include "verifier/verifier.hpp"
#include <limits>
#include <functional>
#include <cmath>
#include <string>
#include <vector>
#include <catch2/generators/catch_generators_range.hpp>
#include <stdexcept>

TEST_CASE("Passing mode as 0 throws invalid_argument exception"){
    auto L = GENERATE(as<double>{},0.7,1.0,100.58,0.1);
    INFO("Built from length: " << L);
    CHECK_THROWS_AS(heat::generate_specific_sin(L,0), std::invalid_argument);
}

TEST_CASE("Negative mode makes entire function negative version of abs(mode)"){
    auto L = GENERATE(as<double>{},0.7,1.0,100.58,0.1);
    auto mode = GENERATE(as<int>{},-1,-2,-3,-4,-10,-20,-100);
    auto x = GENERATE(as<double>{},0.1,0.2,0.5,1.0,5.0);
    INFO("Built from length: " << L << " and mode: " << mode << " and x: " << x);
    CHECK(-1.0*(heat::generate_specific_sin(L,mode)(x)) == heat::generate_specific_sin(L,-mode)(x));
}

TEST_CASE("0, NaN, infinity or negative length throws input validation exception"){
    auto L = GENERATE(as<double>{},0,-0.7,-1.0,-100.58,-0.1, NAN, std::numeric_limits<double>::infinity());
    auto mode = GENERATE(as<int>{},-1,2,-3,4,-10,20,-100);
    INFO("Built from length: " << L << " and mode: " << mode);
    CHECK_THROWS_AS(heat::generate_specific_sin(L,mode),std::invalid_argument);
}

TEST_CASE("Known values match those for given mode"){
    auto L = GENERATE(as<double>{},0.7,1.0,100.58,0.1);
    INFO("Built from length: " << L << " and mode: 1");
    CHECK_THAT(heat::generate_specific_sin(L,1)(L/2),Catch::Matchers::WithinRel(1.0,std::numeric_limits<double>::epsilon()));
    INFO("Built from length " << L << " and mode 2");
    CHECK_THAT(heat::generate_specific_sin(L,2)(L/4),Catch::Matchers::WithinRel(1.0,std::numeric_limits<double>::epsilon()*4));
}

TEST_CASE("Pi used is equal to real calculated pi"){
    CHECK(heat::pi == std::acos(-1.0));
}

