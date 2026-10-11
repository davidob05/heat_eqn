#pragma once
#include <vector>
#include <tuple>
#include <stdexcept>
#include <cstddef>

namespace heat{
    inline constexpr double MAX_N = 1000000000;
    std::vector<double> time_step(const std::vector<double>& curr_state, double r);
    std::tuple<std::size_t,double,double> calculate_step_parameters(double dx, double T, double alpha, double r_req);
    std::vector<double> step_loop(std::vector<double> initial_state, double r, std::size_t n);
}//namespace heat
