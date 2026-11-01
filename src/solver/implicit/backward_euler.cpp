#include <vector>
#include "backward_euler.hpp"

namespace heat{
    void backward_euler(std::vector<double>& curr_state, double r){
        curr_state = std::vector<double>({r,r});
        r = 0.0;
    }
}
