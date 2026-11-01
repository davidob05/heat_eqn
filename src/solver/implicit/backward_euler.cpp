#include <vector>
#include <stdexcept>
#include <cmath>
#include "backward_euler.hpp"
#include "tridiagonal.hpp"

namespace heat{
    void backward_euler(
        std::vector<double>& curr_state, 
        double r
    ){
        if ( curr_state.size() < 3 ){throw std::invalid_argument("Current state must be at least length 3 or more");}

        if ( r < 0 ){throw std::invalid_argument("r must be positive or 0");}

        if ( std::isnan(r) ){throw std::invalid_argument("r cannot be NaN");}

        if ( std::isinf(r) ){throw std::invalid_argument("r must be finite");}

        heat::TridiagonalMatrix mat(std::vector<double>(curr_state.size()-3,-r),std::vector<double>(curr_state.size()-2,1+2*r),std::vector<double>(curr_state.size()-3,-r));
        std::vector<double> sliced_state = std::vector<double>(curr_state.begin() + 1, curr_state.end()-1);
        std::vector<double> solution = solve_tridiagonal(mat,sliced_state);
        
        for ( std::size_t i = 1 ; i < curr_state.size()-1 ; i++ ){
            curr_state[i] = solution[i-1];
        }
    }
}
