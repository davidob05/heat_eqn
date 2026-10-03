#pragma once

namespace heat{

    constexpr double pi = 3.14159265358979323846;
    std::function<double(double)> generate_specific_sin(const double& length, const int& mode);

}//namespace heat
