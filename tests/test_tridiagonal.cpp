#include "solver/implicit/tridiagonal.hpp"
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <stdexcept>
#include <catch2/matchers/catch_matchers_vector.hpp>

TEST_CASE("Invalid sized vectors throw exception in Tridiagonal matrix constructor"){
    CHECK_THROWS_AS(heat::TridiagonalMatrix({-1,-1},{2,2,2},{-1}), std::invalid_argument);
    CHECK_THROWS_AS(heat::TridiagonalMatrix({-1,-1},{2,2},{-1,-1}), std::invalid_argument);
}

TEST_CASE("Valid sized vectors make Tridiagonal matrix properly in the constructor"){
    heat::TridiagonalMatrix mat({-1,-1},{2,2,2},{1,1});
    CHECK_THAT(mat.diag, Catch::Matchers::Equals(std::vector<double>{2,2,2}));
    CHECK_THAT(mat.upper_diag, Catch::Matchers::Equals(std::vector<double>{1,1}));
    CHECK_THAT(mat.lower_diag, Catch::Matchers::Equals(std::vector<double>{-1,-1}));
}

TEST_CASE("Solves a symmetric 3x3 system"){
    heat::TridiagonalMatrix tridiag({-1,-1},{2,2,2},{-1,-1});
    std::vector<double> d({1,0,1});
    std::vector<double> result = heat::solve_tridiagonal(tridiag,d);
    REQUIRE(result.size()==3);
    for( std::size_t i = 0 ; i < 3 ; i++ ){
        CHECK_THAT(result[i],Catch::Matchers::WithinAbs(1.0, 1e-12));
    }
}

TEST_CASE("Tridiagonal solver throws error on mismatched matrix and d sizes"){
    heat::TridiagonalMatrix tridiag({-1,-1},{2,2,2},{-1,-1});
    std::vector<double> d({1,1});
    std::vector<double> e({1,1,1,1});
    CHECK_THROWS_AS(heat::solve_tridiagonal(tridiag,d), std::invalid_argument);
    CHECK_THROWS_AS(heat::solve_tridiagonal(tridiag,e), std::invalid_argument);
}

TEST_CASE("Solves an asymmetric 3x3 system"){
    heat::TridiagonalMatrix tridiag({1,2},{4,5,6},{2,1});
    std::vector<double> d({8,14,22});
    std::vector<double> result = heat::solve_tridiagonal(tridiag,d);
    REQUIRE(result.size()==3);
    for( std::size_t i = 0 ; i < 3 ; i++ ){
        CHECK_THAT(result[i],Catch::Matchers::WithinAbs(static_cast<double>(i)+1, 1e-12));
    }
}
