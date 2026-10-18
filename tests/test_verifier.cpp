#include <algorithm>
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

TEST_CASE("Ensure known error is given by max_error"){
    std::vector<double> calculated({1.0,2.0,3.0});
    std::vector<double> exact({1.0,5.0,2.0});
    CHECK(heat::max_error(calculated,exact) == 3.0);
}

TEST_CASE("0.0 error for identical lists given by max_error"){
    std::vector<double> calculated({1.0,2.0,3.0});
    std::vector<double> exact({1.0,2.0,3.0});
    CHECK(heat::max_error(calculated,exact) == 0.0);
}

TEST_CASE("max_error is commutative"){
    auto calculated = GENERATE(values<std::vector<double>>({
            {3.14, -2.71, 0.58, 42.0, 7.25, -19.83, 100.5},
            {0.001, 12.75, -4.4, 88.12, 3.0, -0.95},
            {-7.5, 21.36, 5.05, 0.0, 64.9, -33.21, 9.99, 1.618},
            {15.2, -8.08, 27.45, 0.33, -61.7},
            {2.5, 2.5, -2.5, 1000.25, 0.1, 47.63, -0.007, 8.88, 19.0}
    }));

    // same length as calculated, but different values
    std::vector<double> exact(calculated);
    std::rotate(exact.begin(), exact.begin() + 1, exact.end());

    INFO("Calculated: " << Catch::Detail::stringify(calculated)
         << " Exact: " << Catch::Detail::stringify(exact));
    CHECK(heat::max_error(calculated,exact) == heat::max_error(exact,calculated));
    CHECK(heat::max_error(calculated,exact)>=0);
}

TEST_CASE("The largest error is positive"){
    std::vector<double> calculated({0.0,5.0,2.0});
    std::vector<double> exact({0.0,0.0,0.0});
    CHECK(heat::max_error(calculated,exact)==5.0);
}

TEST_CASE("Single element lists work"){
    std::vector<double> calculated({2});
    std::vector<double> exact({5});
    CHECK(heat::max_error(calculated,exact)==3.0);
}

TEST_CASE("Different sized lists throw on max_error"){
    std::vector<double> calculated({2,3});
    std::vector<double> exact({5});
    CHECK_THROWS_AS(heat::max_error(calculated,exact),std::invalid_argument);
}

TEST_CASE("Empty lists throw in max_error"){
    std::vector<double> calculated({});
    std::vector<double> exact({});
    CHECK_THROWS_AS(heat::max_error(calculated,exact),std::invalid_argument);
}

TEST_CASE("NaN entries in input vectors throw errors in max_error"){
    auto position = GENERATE(as<std::size_t>{},0,1,2,3,4);
    std::vector<double> calculated({1.0,2.0,3.0,4.0,5.0});
    std::vector<double> exact({1.0,2.0,3.0,4.0,5.0});
    calculated[position] = std::numeric_limits<double>::quiet_NaN();
    INFO("Lists: " << Catch::Detail::stringify(calculated) << " and " << Catch::Detail::stringify(exact));
    CHECK_THROWS_AS(heat::max_error(calculated,exact),std::invalid_argument);
    CHECK_THROWS_AS(heat::max_error(exact,calculated),std::invalid_argument);
    calculated[position] = std::numeric_limits<double>::infinity();
    CHECK_THROWS_AS(heat::max_error(calculated,exact),std::invalid_argument);
    CHECK_THROWS_AS(heat::max_error(exact,calculated),std::invalid_argument);
}

TEST_CASE("Check known exact L2 error, dx = 1"){
    std::vector<double> calculated({0.0,0.0});
    std::vector<double> exact({3.0,4.0});
    double dx = 1.0;
    CHECK(heat::l2_error(calculated,exact,dx)==5.0);
}

TEST_CASE("Check known exact L2 error, dx = 0.25"){
    std::vector<double> calculated({0.0,0.0});
    std::vector<double> exact({3.0,4.0});
    double dx = 0.25;
    CHECK(heat::l2_error(calculated,exact,dx)==2.5);
}

TEST_CASE("Check known exact L2 error, one element"){
    std::vector<double> calculated({0.0});
    std::vector<double> exact({3.0});
    double dx = 4.0;
    CHECK(heat::l2_error(calculated,exact,dx)==6.0);
}

TEST_CASE("Check identical lists give exactly 0.0 error on L2"){
    std::vector<double> calculated({0.0,1.0,3.0,10.0});
    std::vector<double> exact({0.0,1.0,3.0,10.0});
    double dx = 1.0;
    CHECK(heat::l2_error(calculated,exact,dx)==0.0);
}

TEST_CASE("Check L2 is commutative"){
    std::vector<double> calculated({0.1,2.0,3.5,10.75});
    std::vector<double> exact({0.0,1.0,3.0,10.0});
    double dx = 1.0;
    CHECK(heat::l2_error(calculated,exact,dx) == heat::l2_error(exact,calculated,dx));
}


TEST_CASE("Different sized lists throw on L2_error"){
    std::vector<double> calculated({2,3});
    std::vector<double> exact({5});
    double dx = 1.0;
    CHECK_THROWS_AS(heat::l2_error(calculated,exact,dx),std::invalid_argument);
    CHECK_THROWS_AS(heat::l2_error(exact,calculated,dx),std::invalid_argument);

}

TEST_CASE("Empty lists throw in L2_error"){
    std::vector<double> calculated({});
    std::vector<double> exact({});
    double dx = 1.0;
    CHECK_THROWS_AS(heat::l2_error(calculated,exact,dx),std::invalid_argument);
}

TEST_CASE("NaN entries in input vectors throw errors in L2_error"){
    auto position = GENERATE(as<std::size_t>{},0,1,2,3,4);
    std::vector<double> calculated({1.0,2.0,3.0,4.0,5.0});
    std::vector<double> exact({1.0,2.0,3.0,4.0,5.0});
    double dx = 1.0;
    calculated[position] = std::numeric_limits<double>::quiet_NaN();
    INFO("Lists: " << Catch::Detail::stringify(calculated) << " and " << Catch::Detail::stringify(exact));
    CHECK_THROWS_AS(heat::l2_error(calculated,exact,dx),std::invalid_argument);
    CHECK_THROWS_AS(heat::l2_error(exact,calculated,dx),std::invalid_argument);
    calculated[position] = std::numeric_limits<double>::infinity();
    CHECK_THROWS_AS(heat::l2_error(calculated,exact,dx),std::invalid_argument);
    CHECK_THROWS_AS(heat::l2_error(exact,calculated,dx),std::invalid_argument);
}

TEST_CASE("0, negative, NaN or infinite dx throw error on l2_error"){
    std::vector<double> calculated({1.0,2.0,3.0,4.0,5.0});
    std::vector<double> exact({1.0,2.0,3.0,4.0,5.0});
    auto dx = GENERATE(as<double>{},0.0,-1.0,std::numeric_limits<double>::quiet_NaN(),std::numeric_limits<double>::infinity());
    INFO("dx = " << dx);
    CHECK_THROWS_AS(heat::l2_error(calculated,exact,dx),std::invalid_argument);
}

TEST_CASE("Input params giving a gap too large throw error on L2_error"){
    std::vector<double> calculated({0.0});
    std::vector<double> exact({1e200});
    double dx = 1;
    CHECK_THROWS_AS(heat::l2_error(calculated,exact,dx),std::invalid_argument);
}

TEST_CASE("Input params that gap to give less than infinite sum of squares, but infinite once multiplied with dx"){
    std::vector<double> calculated({0.0});
    std::vector<double> exact({1e154});
    double dx = 10;
    CHECK_THROWS_AS(heat::l2_error(calculated,exact,dx),std::invalid_argument);
}
