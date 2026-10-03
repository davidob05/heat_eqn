#include "verifier.hpp"
#include <cmath>
#include <stdexcept>

namespace heat{
    std::function<double(double)> generate_specific_sin(double length, int mode){
        
        if( mode == 0 ) throw std::invalid_argument("Mode for sin(mode*pi*x/L) is 0");
        
        return [length,mode](double x) {
            return std::sin((mode*pi*x)/length);
        };
    }
}
