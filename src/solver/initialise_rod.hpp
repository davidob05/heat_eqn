#pragma once
#include <vector>
#include <cstddef>
#include <functional>
#include "grid.hpp"

namespace heat{
    std::vector<double> initialise_rod(Grid grid, std::function<double (double)> function);
}
