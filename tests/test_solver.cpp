#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include "core/initialise_rod.hpp"
#include "core/grid.hpp"
#include "verifier/verifier.hpp"
#include "solver/time_step.hpp"
#include <limits>
#include <functional>
#include <cmath>
#include <string>
#include <vector>
#include <tuple>
#include <catch2/generators/catch_generators_range.hpp>

TEST_CASE("Rod of 0s stays as rod of 0s"){
    auto r = GENERATE(as<double>{},0.5,0.25,0.6 );
    std::vector<double> orig_vec(10,0);
    std::vector<double> changed_vec = heat::time_step(orig_vec,r);
    CHECK(orig_vec == changed_vec);
}

TEST_CASE("Simple symmetric update updates as expected"){
    std::vector<double> orig_vec(5,0);
    orig_vec[2] = 1;
    std::vector<double> changed_vec = heat::time_step(orig_vec,0.25);
    const std::vector<double> expected = {0, 0.25, 0.5, 0.25, 0};
    CHECK(expected == changed_vec);
}

TEST_CASE("Simple symmetric update updates as expected, with edge r value"){
    std::vector<double> orig_vec(5,0);
    orig_vec[2] = 1;
    std::vector<double> changed_vec = heat::time_step(orig_vec,0.5);
    const std::vector<double> expected = {0.0, 0.5, 0.0, 0.5, 0.0};
    CHECK(expected == changed_vec);
}

TEST_CASE("Sine inscribed rod updates as expected on one update"){
    auto n_intervals = GENERATE(as<std::size_t>{},12,20,1000,49,10000);
    auto L = GENERATE(as<double>{},0.7,1.0,100.58,0.1);
    auto r = GENERATE(as<double>{},0.1, 0.25, 0.4);

    heat::Grid grid(n_intervals,L);
    std::function<double(double)> f = heat::generate_specific_sin(L,1);
    std::vector<double> initial = heat::initialise_rod(grid,f);
    std::vector<double> changed_vec = heat::time_step(initial,r);

    for(std::size_t i = 1; i < initial.size()-1; i++){
        CHECK_THAT(initial[i]*(1-4*r*std::pow(std::sin(heat::pi*grid.dx()/(2*L)),2.0)), Catch::Matchers::WithinAbs(changed_vec[i],10*std::numeric_limits<double>::epsilon()));
    }
}

TEST_CASE("3 timesteps, simple spike tested behaviour"){
    double r = 0.25;
    std::vector<double> orig_vec(5,0);
    orig_vec[2] = 1;
    std::vector<double> changed_vec = heat::time_step(orig_vec,r);
    CHECK(std::vector<double> {0, 0.25, 0.5, 0.25, 0} == changed_vec);
    changed_vec = heat::time_step(changed_vec,r);
    CHECK(std::vector<double> {0, 0.25, 0.375, 0.25, 0} == changed_vec);
    changed_vec = heat::time_step(changed_vec,r);
    CHECK(std::vector<double> {0, 0.21875, 0.3125, 0.21875, 0} == changed_vec);
}

TEST_CASE("Check step parameter calculator gives exact values for known inputs"){
    CHECK(heat::calculate_step_parameters(0.5,1.0,2.0,0.25)==std::make_tuple(std::size_t{32},1.0/32,0.25));
}

TEST_CASE("Step parameter calculator rounds n up when T is not a multiple of the requested dt"){
    const double eps = std::numeric_limits<double>::epsilon();
    auto [n, dt, r_used] = heat::calculate_step_parameters(0.5,1.1,2.0,0.25);
    CHECK(n == 36);
    CHECK_THAT(dt, Catch::Matchers::WithinRel(1.1/36,10*eps));
    CHECK_THAT(r_used, Catch::Matchers::WithinRel(2.0*(1.1/36)/0.25,10*eps));
    CHECK_THAT(r_used, Catch::Matchers::WithinAbs(0.2444,1e-4));
}

TEST_CASE("Step parameter calculator gives a single step when T is smaller than one step"){
    const double eps = std::numeric_limits<double>::epsilon();
    auto [n, dt, r_used] = heat::calculate_step_parameters(0.5,0.001,2.0,0.25);
    CHECK(n == 1);
    CHECK_THAT(dt, Catch::Matchers::WithinRel(0.001,10*eps));
}

TEST_CASE("Step parameter calculator invariants hold over generated inputs"){
    const double eps = std::numeric_limits<double>::epsilon();
    auto dx = GENERATE(as<double>{},0.5,0.1,0.013,2.0);
    auto T = GENERATE(as<double>{},0.001,0.7,1.0,1.1,3.3);
    auto alpha = GENERATE(as<double>{},0.01,1.0,2.0,7.5);
    auto r_req = GENERATE(as<double>{},0.1,0.25,0.4,0.5);

    auto [n, dt, r_used] = heat::calculate_step_parameters(dx,T,alpha,r_req);
    CHECK(n >= 1);
    CHECK_THAT(static_cast<double>(n)*dt, Catch::Matchers::WithinRel(T,10*eps));
    CHECK(r_used <= r_req*(1+10*eps));
}

TEST_CASE("Step parameter calculator rejects bad inputs"){
    auto bad = GENERATE(as<double>{},-1.0,0.0,std::numeric_limits<double>::quiet_NaN(),std::numeric_limits<double>::infinity());

    CHECK_THROWS_AS(heat::calculate_step_parameters(bad,1.0,2.0,0.25),std::invalid_argument);
    CHECK_THROWS_AS(heat::calculate_step_parameters(0.5,bad,2.0,0.25),std::invalid_argument);
    CHECK_THROWS_AS(heat::calculate_step_parameters(0.5,1.0,bad,0.25),std::invalid_argument);
    CHECK_THROWS_AS(heat::calculate_step_parameters(0.5,1.0,2.0,bad),std::invalid_argument);
}

TEST_CASE("Step parameter calculator rejects inputs that push n past the cap"){
    CHECK_THROWS_AS(heat::calculate_step_parameters(1e-200,1.0,2.0,0.25),std::invalid_argument);
    CHECK_THROWS_AS(heat::calculate_step_parameters(1e-5,1.0,2.0,0.25),std::invalid_argument);
    CHECK_THROWS_AS(heat::calculate_step_parameters(0.5,1.0,1e300,0.25),std::invalid_argument);
    CHECK_THROWS_AS(heat::calculate_step_parameters(0.5,1e300,2.0,0.25),std::invalid_argument);
}



