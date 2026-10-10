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



