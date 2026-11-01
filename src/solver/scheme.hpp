#pragma once
#include <tuple>


namespace heat{

    inline constexpr double MAX_N = 1000000000;

    enum Scheme {
        EXPLICIT,
        BACKWARD_EULER,
        CRANK_NICOLSON
    };

    std::tuple<std::size_t,double,double> calculate_step_parameters(double dx, double T, double alpha, double r_req);

}
