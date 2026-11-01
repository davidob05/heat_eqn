#include <vector>
#include <stdexcept>
#include <cmath>
#include "crank_nicolson.hpp"
#include "tridiagonal.hpp"

namespace heat{
    void crank_nicolson(std::vector<double>& curr_state, double r){
        if ( curr_state.size() < 3 ){throw std::invalid_argument("Current state must be at least length 3 or more");}

        if ( r < 0 ){throw std::invalid_argument("r must be positive or 0");}

        if ( std::isnan(r) ){throw std::invalid_argument("r cannot be NaN");}

        if ( std::isinf(r) ){throw std::invalid_argument("r must be finite");}

        heat::TridiagonalMatrix mat(std::vector<double>(curr_state.size()-3,-r/2),std::vector<double>(curr_state.size()-2,1+r),std::vector<double>(curr_state.size()-3,-r/2));
        std::vector<double> param(curr_state.size()-2,0); 
        for ( std::size_t i = 1 ; i < curr_state.size()-1 ; i++ ){
            param[i-1] = (1-r)*curr_state[i] + r/2*(curr_state[i-1] + curr_state[i+1]);
        }
        std::vector<double> solution = solve_tridiagonal(mat,param);
        for ( std::size_t i = 1 ; i < curr_state.size()-1 ; i++ ) {
            curr_state[i] = solution[i-1];
        }
    }
}// namespace heat
