#include "time_step.hpp"
#include <cmath>
#include <stdexcept>
#include <vector>
#include <cstddef>
#include <tuple>


namespace heat{
    void time_step(std::vector<double>& curr_state, double r){
        double prev = curr_state[0];
        
        for(std::size_t i = 1 ; i < curr_state.size() - 1 ; i++){
            const double curr = curr_state[i];
            curr_state[i] = curr + r*(prev - 2.0*curr + curr_state[i+1]);
            prev = curr;
        }
    }

    std::tuple<std::size_t,double,double> calculate_step_parameters(double dx, double T, double alpha, double r_req){
        if( not (dx > 0) || std::isinf(dx)) throw std::invalid_argument("Intervals along rod must be positive and finite");
        if( not (T > 0) || std::isinf(T)) throw std::invalid_argument("Target time be positive and finite");
        if( not (alpha > 0) || std::isinf(alpha)) throw std::invalid_argument("Diffusivity must be positive and finite");
        if( not (r_req > 0) || std::isinf(r_req)) throw std::invalid_argument("Requested r must be positive and finite");

        double dt_req = r_req*dx*dx/alpha;
        if( not (dt_req > 0) || std::isinf(dt_req)) throw std::invalid_argument("Requested r must be positive and finite");
        double n = std::ceil(T/dt_req);
        if( not (n >= 1) || n>MAX_N) throw std::invalid_argument("n must be positive and finite, check input parameters");

        double dt = T/n;
        double r_used = alpha*dt/(dx*dx);
        return {static_cast<std::size_t>(n),dt,r_used};
    }

    std::vector<double> step_loop(std::vector<double> initial_state, double r, std::size_t n){
        for(std::size_t i = 0 ; i<n ; i++){
            time_step(initial_state, r);
        }
        return initial_state;
    }
} // namespace heat
