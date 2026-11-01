#include "time_step.hpp"
#include <cmath>
#include <stdexcept>
#include <vector>
#include <cstddef>
#include <tuple>


namespace heat{
    void time_step(std::vector<double>& curr_state, double r){
        double prev = curr_state[0];
        
        for(std::size_t i = 1 ; i < curr_state.size() - 1 ; i++){
            const double curr = curr_state[i];
            curr_state[i] = curr + r*(prev - 2.0*curr + curr_state[i+1]);
            prev = curr;
        }
    }
} // namespace heat
