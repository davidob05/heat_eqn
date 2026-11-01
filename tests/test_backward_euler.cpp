#include "solver/implicit/backward_euler.hpp"
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
    CHECK(vec[0]==0.0);
    CHECK(vec.back()==0.0);
}

TEST_CASE("Asymmetric input gives asymmetric output"){
    std::vector<double> vec({0,1,0,0});
    heat::backward_euler(vec,1.0);
    CHECK_THAT(vec[1],Catch::Matchers::WithinAbs(0.375,1e-12));
    CHECK_THAT(vec[2],Catch::Matchers::WithinAbs(0.125,1e-12));
    CHECK(vec[0]==0.0);
    CHECK(vec.back()==0.0);
}

TEST_CASE("Three interior points gives correct outputs"){
    std::vector<double> vec({0,1,1,1,0});
    heat::backward_euler(vec,1.0);
    CHECK_THAT(vec[1],Catch::Matchers::WithinAbs(4.0/7.0,1e-12));
    CHECK_THAT(vec[3],Catch::Matchers::WithinAbs(4.0/7.0,1e-12));
    CHECK_THAT(vec[2],Catch::Matchers::WithinAbs(5.0/7.0,1e-12));
    CHECK(vec[0]==0.0);
    CHECK(vec.back()==0.0);
}

TEST_CASE("Ends are untouched"){
    std::vector<double> vec({5.0,1.0,7.0});
    heat::backward_euler(vec,0.5);
    CHECK_THAT(vec[1],Catch::Matchers::WithinAbs(3.5,1e-12));
    CHECK(vec[0] == 5.0);
    CHECK(vec[2]  == 7.0);
}

TEST_CASE("r=0 leaves vector unchanged"){
    std::vector<double> vec({0, 0.3, 0.7, 0});
    heat::backward_euler(vec,0.0);
    CHECK(vec == std::vector<double>({0, 0.3, 0.7, 0}));
}

TEST_CASE("Ends propagate as expected with 0 interior"){
    std::vector<double> vec({2.0, 0.0, 0.0, 0.0, 4.0});
    heat::backward_euler(vec,1.0);
    CHECK_THAT(vec[1],Catch::Matchers::WithinAbs(20.0/21.0,1e-12));
    CHECK_THAT(vec[2],Catch::Matchers::WithinAbs(6.0/7.0,1e-12));
    CHECK_THAT(vec[3],Catch::Matchers::WithinAbs(34.0/21.0,1e-12));
    CHECK(vec[0] == 2.0);
    CHECK(vec[4] == 4.0);
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

TEST_CASE("Backward Euler throws errors on bad inputs"){
    std::vector<double> vec({0,1,0});
    std::vector<double> vec_c = vec;

    std::vector<double> small_vec2({1,1});
    std::vector<double> vec2_c = small_vec2;

    std::vector<double> small_vec1({1});
    std::vector<double> vec1_c = small_vec1;

    std::vector<double> small_vec0({});
    std::vector<double> vec0_c = small_vec0;

    CHECK_THROWS_AS(heat::backward_euler(small_vec2,0.5),std::invalid_argument);
    CHECK(small_vec2==vec2_c);
    CHECK_THROWS_AS(heat::backward_euler(small_vec1,0.5),std::invalid_argument);
    CHECK(small_vec1==vec1_c);
    CHECK_THROWS_AS(heat::backward_euler(small_vec0,0.5),std::invalid_argument);
    CHECK(small_vec0==vec0_c);

    CHECK_THROWS_AS(heat::backward_euler(vec,-0.5),std::invalid_argument);
    CHECK(vec==vec_c);
    CHECK_THROWS_AS(heat::backward_euler(vec,std::numeric_limits<double>::infinity()),std::invalid_argument);
    CHECK(vec==vec_c);
    CHECK_THROWS_AS(heat::backward_euler(vec,std::numeric_limits<double>::quiet_NaN()),std::invalid_argument);
    CHECK(vec==vec_c);
}

