#include "solver/implicit/backward_euler.hpp"
#include "solver/implicit/tridiagonal.hpp"
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <stdexcept>
#include <catch2/matchers/catch_matchers_vector.hpp>

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
