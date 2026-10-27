#include <vector>
#include "tridiagonal.hpp"

namespace heat{
    std::vector<double> solve_tridiagonal(
        const TridiagonalMatrix& tri_mat, 
        const std::vector<double>& d
    ){
        if ( d.size() != tri_mat.diag.size() ){
            throw std::invalid_argument("Right hand side vector needs to match diagonal length of tridiagonal");
        }
        std::vector<double> diag_work = tri_mat.diag;
        std::vector<double> x = d;
        std::size_t n = tri_mat.diag.size();

        for ( std::size_t i = 1 ; i < n ; i++ ){
            double multiplier = tri_mat.lower_diag[i-1]/diag_work[i-1];
            diag_work[i]-= multiplier*tri_mat.upper_diag[i-1];
            x[i] -= multiplier*x[i-1];
        }

        x[n-1] /= diag_work[n-1];

        for ( std::size_t i = n - 1 ; i > 0 ; i-- ){
            x[i-1] = (x[i-1]-(x[i]*tri_mat.upper_diag[i-1]))/diag_work[i-1];
        }

        return x;
    }

}
