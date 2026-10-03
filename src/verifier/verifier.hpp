#pragma once
#include <functional>

namespace heat{

    constexpr double pi = 3.14159265358979323846;
    std::function<double(double)> generate_specific_sin(double length, int mode);

}//namespace heat
