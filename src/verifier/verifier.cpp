#include "verifier.hpp"
#include <cmath>
#include <functional>

namespace heat{
    std::function<double(double)> generate_specific_sin(double length, int mode){
        return [length,mode](double x) {
            return std::sin((mode*pi*x)/length);
        };
    }
}
