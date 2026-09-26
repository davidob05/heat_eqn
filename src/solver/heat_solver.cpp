#include "heat_solver.hpp"
namespace heat {
    std::vector<double> init_rod(std::size_t length){
        std::vector<double> vec(length,0.0);
        for(std::size_t i = 1;i+1<length;i++){
            vec[i] = 1.0;
        }
        return vec;
    }
}
