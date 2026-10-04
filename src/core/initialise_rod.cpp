#include "initialise_rod.hpp"
#include <vector>
#include <tuple>
#include <functional>
#include <cmath>

namespace heat{
    std::vector<double> initialise_rod(const Grid& grid, std::function<double (double)> function){
        const std::vector<double>& positions = grid.x();

        const std::size_t vec_len = positions.size();
        std::vector<double> rod(vec_len,0);
        for(std::size_t i = 1 ; i <vec_len - 1 ; i++){
            rod[i] = function(positions[i]);
        }
        return rod;
    }

    std::tuple<bool,bool> check_fn_0_endpoints(const Grid& grid, std::function<double (double)> function){
        bool left = false;
        bool right = false;
        if(std::abs(function(grid.x()[0]))>16*std::numeric_limits<double>::epsilon()){
            left = true;
        }
        if(std::abs(function(grid.x().back()))>16.0*std::numeric_limits<double>::epsilon()){
            right = true;
        }

        return std::tuple(left,right);
    }

}
