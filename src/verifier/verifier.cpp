#include "verifier.hpp"
#include <cmath>
#include <stdexcept>

namespace heat{
    std::function<double(double)> generate_specific_sin(double length, int mode){
        
        if( mode == 0 ) throw std::invalid_argument("Mode for sin(mode*pi*x/L) is 0");
        if( not (length > 0) || std::isinf(length)) throw std::invalid_argument("Length of rod must be positive, defined and not infinite");

        return [length,mode](double x) {
            return std::sin((mode*pi*x)/length);
        };
    }
}
