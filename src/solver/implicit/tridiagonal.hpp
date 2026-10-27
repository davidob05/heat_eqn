#pragma once
#include <vector>
#include <stdexcept>
#include <utility>

namespace heat{
    struct TridiagonalMatrix{
        std::vector<double> lower_diag;
        std::vector<double> diag;
        std::vector<double> upper_diag;
        
        TridiagonalMatrix(
            std::vector<double> lower,
            std::vector<double> middle,
            std::vector<double> upper 
        ) : 
        lower_diag(std::move(lower)), 
        diag(std::move(middle)), 
        upper_diag(std::move(upper))
        {}
    };

    std::vector<double> solve_tridiagonal(
        const TridiagonalMatrix& tri_mat, 
        const std::vector<double>& d
    );
}
