#include <vector>
#include "tridiagonal.hpp"

namespace heat{
    std::vector<double> solve_tridiagonal(
        const TridiagonalMatrix& tri_mat, 
        const std::vector<double>& d
    ){
        std::vector<double> vec(d.size(),0.0);
        return vec;
    }

}
