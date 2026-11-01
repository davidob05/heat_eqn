#pragma once
#include <vector>

namespace heat{
    void crank_nicolson(std::vector<double>& curr_state, double r);
}
