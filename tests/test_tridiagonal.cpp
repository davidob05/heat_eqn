#include "solver/implicit/tridiagonal.hpp"
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <stdexcept>


TEST_CASE("Solves a symmetric 3x3 system"){
    heat::TridiagonalMatrix tridiag({-1,-1},{2,2,2},{-1,-1});
    std::vector<double> d({1,0,1});
    std::vector<double> result = heat::solve_tridiagonal(tridiag,d);
    REQUIRE(result.size()==3);
    for( std::size_t i = 0 ; i < 3 ; i++ ){
        CHECK_THAT(result[i],Catch::Matchers::WithinAbs(1.0, 1e-12));
    }
}

TEST_CASE("Invalid sized vectors throw exception in Tridiagonal matrix constructor"){
    CHECK_THROWS_AS(heat::TridiagonalMatrix({-1,-1},{2,2,2},{-1}), std::invalid_argument);
    CHECK_THROWS_AS(heat::TridiagonalMatrix({-1,-1},{2,2},{-1,-1}), std::invalid_argument);
}
