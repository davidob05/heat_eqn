#include "initialise_rod.hpp"
#include <cmath>

namespace heat{
    std::vector<double> initialise_rod(const Grid& grid, std::function<double (double)> function){
        const std::vector<double>& positions = grid.x();

        //TODO: implement warning

        const std::size_t vec_len = positions.size();
        std::vector<double> rod(vec_len,0);
        for(std::size_t i = 1 ; i <vec_len - 1 ; i++){
            rod[i] = function(positions[i]);
        }
        return rod;
    }
}
