#pragma once
#include <vector>
#include <cstddef>
#include <functional>
#include <tuple>
#include "grid.hpp"

namespace heat{
    std::vector<double> initialise_rod(const Grid& grid, std::function<double (double)> function);
    std::tuple<bool,bool> check_fn_0_endpoints(const Grid&, std::function<double (double)> function);
}
