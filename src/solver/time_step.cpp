#include "time_step.hpp"

namespace heat{
    std::vector<double> time_step(const std::vector<double>& buffer, double r){
        std::vector<double> returned_state = buffer;
        for(std::size_t i = 1; i < buffer.size() - 1 ; i++){
            returned_state[i] = buffer[i] + r*(buffer[i-1] - 2.0*buffer[i] + buffer[i+1]);
        }
        return returned_state;
    }
} // namespace heat
