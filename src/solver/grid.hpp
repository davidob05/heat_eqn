#pragma once
#include <vector>
#include <cstddef>

namespace heat{

    class Grid{
        public:
            Grid(std::size_t n_intervals, double length);
            double dx() const { return delta_x; }
            std::size_t size() const { return positions.size(); }
            const std::vector<double>& x() const {return positions; }

        private:
            double delta_x;
            std::vector<double> positions;
    };

}//namespace heat
