#include "solver/implicit/crank_nicolson.hpp"
#include "verifier/verifier.hpp"
#include "core/initialise_rod.hpp"
#include "core/grid.hpp"
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <catch2/generators/catch_generators_range.hpp>
#include <stdexcept>
#include <cmath>
#include <vector>
#include <functional>
#include <limits>

TEST_CASE("Single interior point kills wrong main diagonal Crank Nicholson"){
    std::vector<double> vec({0.0,1.0,0.0});
    heat::crank_nicolson(vec,0.5);
    CHECK(vec[0] == 0.0);
    CHECK(vec[2] == 0.0);
    CHECK_THAT(vec[1], Catch::Matchers::WithinAbs(1.0/3.0,1e-12));
}

TEST_CASE("Symmetric pair gives symmetric output Crank Nicholson"){
    std::vector<double> vec({0.0,1.0,1.0,0.0});
    heat::crank_nicolson(vec,1.0);
    CHECK(vec[0] == 0.0);
    CHECK(vec[3] == 0.0);
    CHECK_THAT(vec[1], Catch::Matchers::WithinAbs(1.0/3.0,1e-12));
    CHECK_THAT(vec[2], Catch::Matchers::WithinAbs(1.0/3.0,1e-12));
}

TEST_CASE("Asymmetric interior gives asymmetric output Crank Nicholson"){
    std::vector<double> vec({0.0,1.0,0.0,0.0});
    heat::crank_nicolson(vec,1.0);
    CHECK(vec[0] == 0.0);
    CHECK(vec[3] == 0.0);
    CHECK_THAT(vec[1], Catch::Matchers::WithinAbs(1.0/15.0,1e-12));
    CHECK_THAT(vec[2], Catch::Matchers::WithinAbs(4.0/15.0,1e-12));
}

TEST_CASE("5 Valued vector gives expected output Crank Nicholson"){
    std::vector<double> vec({0.0,1.0,1.0,1.0,0.0});
    heat::crank_nicolson(vec,1.0);
    CHECK(vec[0] == 0.0);
    CHECK(vec[4] == 0.0);
    CHECK_THAT(vec[1], Catch::Matchers::WithinAbs(3.0/7.0,1e-12));
    CHECK_THAT(vec[2], Catch::Matchers::WithinAbs(5.0/7.0,1e-12));
    CHECK_THAT(vec[3], Catch::Matchers::WithinAbs(3.0/7.0,1e-12));
}

TEST_CASE("Endpoints are unchanged Crank Nicholson"){
    std::vector<double> vec({5.0,1.0,7.0});
    heat::crank_nicolson(vec,0.5);
    CHECK(vec[0] == 5.0);
    CHECK(vec[2] == 7.0);
    CHECK_THAT(vec[1], Catch::Matchers::WithinAbs(1.0/3.0,1e-12));
}
TEST_CASE("r=0 leaves vector unchanged Crank Nicholson"){
    std::vector<double> vec({0.0, 0.3, 0.7, 0.0});
    heat::crank_nicolson(vec,0.0);
    CHECK(vec == std::vector<double>({0.0, 0.3, 0.7, 0.0}));
}

TEST_CASE("Expected behaviour holds for sin function initialiser for 5 steps, modes 1, 2, 3, Crank Nicolson"){
    int mode = GENERATE(as<int>{},1,2,3);
    double length = 1.0;
    double r = 5.0;
    std::size_t n = 5;
    std::size_t iterations = 5;

    heat::Grid grid(n,length);
    std::function<double(double)> func = heat::generate_specific_sin(length,mode);
    std::vector<double> result = heat::initialise_rod(grid,func);
    std::vector<double> initial = result;
    double dx = grid.dx();
    double s = std::sin(mode*heat::pi*dx/(2*length))*std::sin(mode*heat::pi*dx/(2*length));
    double expected_fact = (1 - 2*r*s) / (1 + 2*r*s);
    for ( std::size_t j = 1; j < iterations + 1 ; j++){
        heat::crank_nicolson(result,r);
        REQUIRE(result.size()==initial.size());
        for ( std::size_t i = 1 ; i < grid.size() - 1 ; i++){
            CHECK_THAT(result[i], Catch::Matchers::WithinAbs(initial[i]*std::pow(expected_fact,j),1e-12));
        }
        CHECK(result[0]==initial[0]);
        CHECK(result.back()==initial.back());
    }
}

TEST_CASE("Crank Nicolson throws errors on bad inputs"){
    std::vector<double> vec({0.0,1.0,0.0});
    std::vector<double> vec_c = vec;

    std::vector<double> small_vec2({1.0,1.0});
    std::vector<double> vec2_c = small_vec2;

    std::vector<double> small_vec1({1.0});
    std::vector<double> vec1_c = small_vec1;

    std::vector<double> small_vec0({});
    std::vector<double> vec0_c = small_vec0;

    CHECK_THROWS_AS(heat::crank_nicolson(small_vec2,0.5),std::invalid_argument);
    CHECK(small_vec2==vec2_c);
    CHECK_THROWS_AS(heat::crank_nicolson(small_vec1,0.5),std::invalid_argument);
    CHECK(small_vec1==vec1_c);
    CHECK_THROWS_AS(heat::crank_nicolson(small_vec0,0.5),std::invalid_argument);
    CHECK(small_vec0==vec0_c);

    CHECK_THROWS_AS(heat::crank_nicolson(vec,-0.5),std::invalid_argument);
    CHECK(vec==vec_c);
    CHECK_THROWS_AS(heat::crank_nicolson(vec,std::numeric_limits<double>::infinity()),std::invalid_argument);
    CHECK(vec==vec_c);
    CHECK_THROWS_AS(heat::crank_nicolson(vec,std::numeric_limits<double>::quiet_NaN()),std::invalid_argument);
    CHECK(vec==vec_c);
}

