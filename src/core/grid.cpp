#include "grid.hpp"
#include <stdexcept>
#include <cmath>

namespace heat{

    Grid::Grid(std::size_t n_intervals, double length)
        : delta_x(length / static_cast<double>(n_intervals)),
        positions(n_intervals + 1),
        len(length)
    {
        if( not (length > 0) || std::isinf(length)) throw std::invalid_argument("Length of rod must be positive, defined and not infinite");
        if( n_intervals < 1 ) throw std::invalid_argument("Grid needs at least 1 interval");
        for( std::size_t i = 0 ; i < n_intervals + 1 ; i++ ){
            positions[i] = (static_cast<double>(i)/n_intervals)*length;
        }
    }
}//namespace heat
