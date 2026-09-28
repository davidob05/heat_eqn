#include "grid.hpp"
#include <stdexcept>

namespace heat{

    Grid::Grid(std::size_t n, double length)
        : delta_x(length / static_cast<double>(n - 1)),
          positions(n)
    {
        if( n < 2 ) throw std::invalid_argument("Grid needs at least 2 points");
        for( std::size_t i = 0 ; i < n ; i++ ){
            positions[i] = static_cast<double>(i)*delta_x;
        }
    }
}//namespace heat
