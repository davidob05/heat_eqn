#include <vector>
#include <stdexcept>
#include <cmath>
#include "crank_nicolson.hpp"
#include "tridiagonal.hpp"

namespace heat{
    void crank_nicolson(std::vector<double>& curr_state, double r){
        curr_state = std::vector<double>({r,r});
    }

} // namespace heat
