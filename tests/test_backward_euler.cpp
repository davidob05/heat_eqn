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
    heat::backward_euler(vec,0.5);
    CHECK(vec == std::vector<double>({0.0,0.5,0.5,0.0}));
}

TEST_CASE("Assymetric input gives asymmetric output"){
    std::vector<double> vec({0,1,0});
    heat::backward_euler(vec,0.5);
    CHECK(vec == std::vector<double>({0.0,0.375,0.125,0.0}));
}
