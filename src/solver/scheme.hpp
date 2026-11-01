#pragma once
#include <tuple>
#include <vector>
#include <cstddef>
#include <vector>

namespace heat{

    inline constexpr double MAX_N = 1000000000;

    enum class Scheme {
        EXPLICIT,
        BACKWARD_EULER,
        CRANK_NICOLSON
    };

    std::tuple<std::size_t,double,double> calculate_step_parameters(double dx, double T, double alpha, double r_req);
    void advance_state(std::vector<double>& state, double r, std::size_t n, Scheme scheme);
}
