#include "time_step.hpp"
#include <cmath>
#include <stdexcept>
#include <vector>
#include <cstddef>
#include <tuple>


namespace heat{
    std::vector<double> time_step(const std::vector<double>& buffer, double r){
        std::vector<double> returned_state = buffer;
        for(std::size_t i = 1; i < buffer.size() - 1 ; i++){
            returned_state[i] = buffer[i] + r*(buffer[i-1] - 2.0*buffer[i] + buffer[i+1]);
        }
        return returned_state;
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
        std::vector<double> copy = initial_state;
        for(std::size_t i = 0 ; i<n ; i++){
            initial_state = time_step(initial_state, r);
        }
        return initial_state;
    }

} // namespace heat
