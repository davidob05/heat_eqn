#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include "verifier/verifier.hpp"
#include "core/initialise_rod.hpp"
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

TEST_CASE("t=0 means the final solution is equal to the initial state"){
    auto L = GENERATE(as<double>{},0.7,1.0,100.58,0.1);
    auto mode = GENERATE(as<int>{},-1, 2, 3, -2);
    auto alpha = GENERATE(as<double>{},0.0,0.1, 1.0, 2.5, 1.1e-4);
    auto n_intervals = GENERATE(as<std::size_t>{},12,20,1000,1,49);

    INFO("Built from length: " << L << " and mode: " << mode << " and alpha: " << alpha << " and intervals: " << n_intervals);
    heat::Grid grid(n_intervals,L);
    std::function<double(double)> f = heat::generate_specific_sin(L,mode);

    std::vector<double> initial = heat::initialise_rod(grid,f);
    std::vector<double> solution = heat::exact_soln(grid,mode,0,alpha);
    REQUIRE(initial.size() == solution.size());
    for(std::size_t i = 0 ; i < solution.size() ; i++){
        CHECK(initial[i] == solution[i]);
    }
}

TEST_CASE("Factor is applied correctly to each point"){

    auto tau = GENERATE(as<double>{},0.001, 0.01, 0.05);
    auto L = GENERATE(as<double>{},0.7,1.0,100.58,0.1);
    auto mode = GENERATE(as<int>{},-1, 2, 3, -2);
    auto alpha = GENERATE(as<double>{},0.1, 1.0, 2.5, 1.1e-4);
    auto n_intervals = GENERATE(as<std::size_t>{},12,20,1000,1,49);

    INFO("Built from length: " << L << " and mode: " << mode << " and alpha: " << alpha << " and tau: " << tau << " and intervals: " << n_intervals);
    double t = tau * L * L / alpha;
    heat::Grid grid(n_intervals,L);
    std::function<double(double)> f = heat::generate_specific_sin(L,mode);
    std::vector<double> initial = heat::initialise_rod(grid,f);
    double expected_factor = std::exp(-mode*mode*tau*heat::pi*heat::pi);
    std::vector<double> solution = heat::exact_soln(grid,mode,t,alpha);
    for(std::size_t i = 0 ; i < solution.size() ; i++){
        CHECK_THAT(solution[i],Catch::Matchers::WithinRel(expected_factor*initial[i],8.0*std::numeric_limits<double>::epsilon()));
    }
}

TEST_CASE("Handles known case: 1/e check"){

    auto n_intervals = GENERATE(as<std::size_t>{},4,8,12,16,20,4000);

    INFO("Built from intervals: " << n_intervals);
    heat::Grid grid(n_intervals,3);
    std::vector<double> solution = heat::exact_soln(grid,2,9.0/(4.0*heat::pi*heat::pi),1);
    CHECK_THAT(solution[n_intervals/4],Catch::Matchers::WithinRel(std::exp(-1),std::numeric_limits<double>::epsilon()));
}

TEST_CASE("Starts and ends with 0"){

    auto tau = GENERATE(as<double>{},0.001, 0.01, 0.05);
    auto L = GENERATE(as<double>{},0.7,1.0,100.58,0.1);
    auto mode = GENERATE(as<int>{},-1, 2, 3, -2);
    auto alpha = GENERATE(as<double>{},0.1, 1.0, 2.5, 1.1e-4);
    auto n_intervals = GENERATE(as<std::size_t>{},12,20,1000,1,49);

    INFO("Built from length: " << L << " and mode: " << mode << " and alpha: " << alpha << " and tau: " << tau << " and intervals: " << n_intervals);
    double t = tau * L * L / alpha;
    heat::Grid grid(n_intervals,L);
    std::vector<double> solution = heat::exact_soln(grid,mode,t,alpha);
    CHECK(solution[0] == 0);
    CHECK(solution.back() == 0);
}

TEST_CASE("Bad inputs throw input_validation exception"){
    auto L = GENERATE(as<double>{},0.7,1.0,100.58,0.1);
    auto n_intervals = GENERATE(as<std::size_t>{},12,20,1000,1,49);

    INFO("Built from length: " << L << " and intervals: " << n_intervals);
    heat::Grid grid(n_intervals,L);
    CHECK_THROWS_AS(heat::exact_soln(grid,1,1.0,-1.0),std::invalid_argument);
    CHECK_THROWS_AS(heat::exact_soln(grid,1,1.0,NAN),std::invalid_argument);
    CHECK_THROWS_AS(heat::exact_soln(grid,1,1.0,std::numeric_limits<double>::infinity()),std::invalid_argument);

    CHECK_THROWS_AS(heat::exact_soln(grid,1,-1.0,1.0),std::invalid_argument);
    CHECK_THROWS_AS(heat::exact_soln(grid,1,NAN,1.0),std::invalid_argument);
    CHECK_THROWS_AS(heat::exact_soln(grid,1,std::numeric_limits<double>::infinity(),1.0),std::invalid_argument);
}
