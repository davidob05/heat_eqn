#include "solver/implicit/backward_euler.hpp"
#include "verifier/verifier.hpp"
#include "core/initialise_rod.hpp"
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <catch2/generators/catch_generators_range.hpp>
#include <stdexcept>
#include <cmath>

TEST_CASE("Single interior point kills wrong main diagonal"){
    std::vector<double> vec({0,1,0});
    heat::backward_euler(vec,0.5);
    CHECK(vec == std::vector<double>({0.0,0.5,0.0}));
}

TEST_CASE("Symmetric pair gives symmetric output"){
    std::vector<double> vec({0,1,1,0});
    heat::backward_euler(vec,1.0);
    CHECK_THAT(vec[1],Catch::Matchers::WithinAbs(0.5,1e-12));
    CHECK_THAT(vec[2],Catch::Matchers::WithinAbs(0.5,1e-12));
}

TEST_CASE("Assymetric input gives asymmetric output"){
    std::vector<double> vec({0,1,0,0});
    heat::backward_euler(vec,1.0);
    CHECK_THAT(vec[1],Catch::Matchers::WithinAbs(0.375,1e-12));
    CHECK_THAT(vec[2],Catch::Matchers::WithinAbs(0.125,1e-12));
}

TEST_CASE("Three interior points gives correct outputs"){
    std::vector<double> vec({0,1,1,1,0});
    heat::backward_euler(vec,1.0);
    CHECK_THAT(vec[1],Catch::Matchers::WithinAbs(4.0/7.0,1e-12));
    CHECK_THAT(vec[3],Catch::Matchers::WithinAbs(4.0/7.0,1e-12));
    CHECK_THAT(vec[2],Catch::Matchers::WithinAbs(5.0/7.0,1e-12));
}

TEST_CASE("Ends are untouched"){
    std::vector<double> vec({5.0,1.0,7.0});
    heat::backward_euler(vec,0.5);
    CHECK(vec == std::vector<double>({5.0,0.5,7.0}));
}

TEST_CASE("r=0 leaves vector unchanged"){
    std::vector<double> vec({0, 0.3, 0.7, 0});
    heat::backward_euler(vec,0.0);
    CHECK(vec == std::vector<double>({0, 0.3, 0.7, 0}));
}

TEST_CASE("Expected behaviour holds for sin function initialiser for 10 steps, modes 1, 2, 3"){
    int mode = GENERATE(as<int>{},1,2,3);
    double length = 1.0;
    double r = 5.0;
    std::size_t n = 10;
    std::size_t iterations = 10;

    heat::Grid grid(n,length);
    std::function<double(double)> func = heat::generate_specific_sin(length,mode);
    std::vector<double> result = heat::initialise_rod(grid,func);
    std::vector<double> initial = result;
    double dx = grid.dx();
    double expected_fact = 1.0/(1+4.0*r*std::sin(mode*heat::pi*dx/(2*length))*std::sin(mode*heat::pi*dx/(2*length)));
    for ( std::size_t j = 1; j < iterations + 1 ; j++){
        heat::backward_euler(result,r);
        REQUIRE(result.size()==initial.size());
        for ( std::size_t i = 1 ; i < grid.size() - 1 ; i++){
            CHECK_THAT(result[i], Catch::Matchers::WithinAbs(initial[i]*std::pow(expected_fact,j),1e-12));
        }
        CHECK(result[0]==initial[0]);
        CHECK(result.back()==initial.back());
    }
}
