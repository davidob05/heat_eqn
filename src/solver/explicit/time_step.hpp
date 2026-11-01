#pragma once
#include <vector>
#include <tuple>
#include <stdexcept>
#include <cstddef>

namespace heat{
    // Advances the rod by one explicit (FTCS) time step, in place.
    // Preconditions, not checked for speed: curr_state.size() >= 2, and r comes from
    // calculate_step_parameters (finite, non-negative).
    // The first and last elements are never modified.
    // The argument is overwritten; pass a copy if the old state is still needed.
    void time_step(std::vector<double>& curr_state, double r);
    // Applies n time steps to state, in place. Same preconditions as time_step.
    void step_loop(std::vector<double>& state, double r, std::size_t n);
}//namespace heat
