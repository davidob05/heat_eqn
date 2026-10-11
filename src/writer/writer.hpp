#pragma once
#include <vector>
#include <iostream>
#include <fstream>
#include <functional>
#include <tuple>
#include "core/grid.hpp"

namespace heat{
    void write_to_stream(std::ostream& stream, double t, const Grid& grid, const std::vector<double>& calculated, const std::vector<double>& exact);
    void write_header(std::ostream& stream);
}
