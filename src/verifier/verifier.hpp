#pragma once
#include <functional>
#include <vector>
#include "core/grid.hpp"

namespace heat{

    constexpr double pi = 3.14159265358979323846;
    std::function<double(double)> generate_specific_sin(double length, int mode);
    std::vector<double> exact_soln(const Grid& grid, int mode, double t, double alpha);
}//namespace heat
