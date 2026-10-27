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
        {
            if ( lower_diag.size() != upper_diag.size() || diag.size() != upper_diag.size() + 1){
                throw std::invalid_argument("Diagonal of length n requires both adjacent matrix rows to have n-1 length");
            }
        }
    };

    std::vector<double> solve_tridiagonal(
        const TridiagonalMatrix& tri_mat, 
        const std::vector<double>& d
    );
}
