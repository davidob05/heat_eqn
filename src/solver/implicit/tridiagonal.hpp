#pragma once
#include <vector>

namespace heat{
    struct TridiagonalMatrix{
        std::vector<double> lower_diag;
        std::vector<double> diag;
        std::vector<double> upper_diag;
        
        TridiagonalMatrix(
            std::vector<double> lower,
            std::vector<double> middle,
            std::vector<double> upper 
        ){
            lower_diag=lower;diag=middle;upper_diag=upper;
        }
    };

    std::vector<double> solve_tridiagonal(
        const TridiagonalMatrix& tri_mat, 
        const std::vector<double>& d
    );
}
