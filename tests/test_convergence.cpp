#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include "core/grid.hpp"
#include "core/initialise_rod.hpp"
#include "solver/time_step.hpp"
#include "verifier/verifier.hpp"
#include <vector>
#include <cmath>
#include <cstddef>

TEST_CASE("Explicit scheme converges at r = 0.4","[convergence]"){
    const int mode = 1;
    const double L = 1.0;
    const double alpha = 1.0;
    const double T = 0.1;
    const double r_req = 0.4;

    std::vector<std::size_t> n_intervals = {20,40,80,160,320};
    std::vector<double> errors = {0.0,0.0,0.0,0.0,0.0};
    
    for ( std::size_t i = 0 ; i < n_intervals.size() ; i++ ){
        INFO("N = " << n_intervals[i]);
        heat::Grid grid(n_intervals[i],L);
        auto [n, dt, r_used ] = heat::calculate_step_parameters(grid.dx(),T,alpha,r_req);
        REQUIRE_THAT(T,Catch::Matchers::WithinRel(static_cast<double>(n)*dt,1e-9));
        REQUIRE_THAT(r_used,Catch::Matchers::WithinRel(r_req,1e-9));
        REQUIRE(n==n_intervals[i]*n_intervals[i]/4);//should hold since T/dt_req = 0.1, N^2/0.4 exactly, if it fails ciel increases n up by 1
        std::vector<double> rod = heat::initialise_rod(grid, heat::generate_specific_sin(L,mode));
        heat::step_loop(rod,r_used,n);
        std::vector<double> exact = heat::exact_soln(grid,mode,T,alpha);
        errors[i] = heat::max_error(rod,exact);
    }

    for( std::size_t i = 1 ; i < errors.size() ; i++ ){
        REQUIRE(errors[i]>0.0);
        double p = std::log(errors[i-1]/errors[i])/std::log(2.0);
        INFO("Order between between number of intervals: " << n_intervals[i-1] << " and " << n_intervals[i] << " giving p = " << p);
        CHECK_THAT(p, Catch::Matchers::WithinAbs(2.0,0.1));
        CHECK(errors[i] < errors[i-1]);
    }
}
