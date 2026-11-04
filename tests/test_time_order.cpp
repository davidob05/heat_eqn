#include "solver/implicit/crank_nicolson.hpp"
#include "verifier/verifier.hpp"
#include "core/initialise_rod.hpp"
#include "core/grid.hpp"
#include "solver/scheme.hpp"
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <catch2/generators/catch_generators_range.hpp>
#include <stdexcept>
#include <cmath>
#include <vector>
#include <functional>
#include <limits>

TEST_CASE("Check order of Crank Nicolson method"){
    double length = 1.0;
    double alpha = 1.0;
    double T = 0.1;
    int mode = 1;
    std::size_t N = 2000;
    std::vector<double> r_reqs = {16000.0, 8000.0, 4000.0};
    std::vector<std::size_t> expected_step_counts = {25,50,100};
    std::vector<double> errors(3,0.0);
    std::vector<double> p(errors.size()-1,0.0);
    heat::Grid grid(N,length);
    std::function<double(double)> func = heat::generate_specific_sin(length,mode);
    for (std::size_t i = 0 ; i < r_reqs.size() ; i++ ){
        std::vector<double> vec = heat::initialise_rod(grid,func);
        auto [n,dt,r_used] = heat::calculate_step_parameters(grid.dx(),T,alpha,r_reqs[i]);
        REQUIRE_THAT(T,Catch::Matchers::WithinRel(static_cast<double>(n)*dt,1e-9));
        REQUIRE_THAT(r_used,Catch::Matchers::WithinRel(r_reqs[i],1e-9));
        REQUIRE(expected_step_counts[i]==n);
        heat::advance_state(vec,r_used,n,heat::Scheme::CRANK_NICOLSON);
        std::vector<double> exact = heat::exact_soln(grid,mode,T,alpha);
        errors[i] = heat::max_error(vec,exact);
        REQUIRE(errors[i]>0);
    }

    for (std::size_t i = 1 ; i<errors.size() ; i++){
        CHECK(errors[i]<errors[i-1]);
        p[i-1] =std::log(errors[i-1]/errors[i])/std::log(2.0);
        CHECK_THAT(p[i-1],Catch::Matchers::WithinAbs(2.0,0.1));
    }
}

TEST_CASE("Check order of Backward Euler method"){
    double length = 1.0;
    double alpha = 1.0;
    double T = 0.1;
    int mode = 1;
    std::size_t N = 2000;
    std::vector<double> r_reqs = {16000.0, 8000.0, 4000.0, 2000.0};
    std::vector<std::size_t> expected_step_counts = {25,50,100,200};
    std::vector<double> errors(4,0.0);
    std::vector<double> p(errors.size()-1,0.0);
    heat::Grid grid(N,length);
    std::function<double(double)> func = heat::generate_specific_sin(length,mode);
    for (std::size_t i = 0 ; i < r_reqs.size() ; i++ ){
        std::vector<double> vec = heat::initialise_rod(grid,func);
        auto [n,dt,r_used] = heat::calculate_step_parameters(grid.dx(),T,alpha,r_reqs[i]);
        REQUIRE_THAT(T,Catch::Matchers::WithinRel(static_cast<double>(n)*dt,1e-9));
        REQUIRE_THAT(r_used,Catch::Matchers::WithinRel(r_reqs[i],1e-9));
        REQUIRE(expected_step_counts[i]==n);
        heat::advance_state(vec,r_used,n,heat::Scheme::BACKWARD_EULER);
        std::vector<double> exact = heat::exact_soln(grid,mode,T,alpha);
        errors[i] = heat::max_error(vec,exact);
        REQUIRE(errors[i]>0);
    }

    for (std::size_t i = 1 ; i<errors.size() ; i++){
        CHECK(errors[i]<errors[i-1]);
        p[i-1] =std::log(errors[i-1]/errors[i])/std::log(2.0);
        CHECK_THAT(p[i-1],Catch::Matchers::WithinAbs(1.0,0.1));
    }
}
