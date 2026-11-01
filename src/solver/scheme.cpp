#include "scheme.hpp"
#include "solver/explicit/time_step.hpp"
#include "solver/implicit/backward_euler.hpp"
#include "solver/implicit/crank_nicolson.hpp"
#include <tuple>
#include <vector>
#include <cmath>
#include <stdexcept>


namespace heat{

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

    void advance_state(std::vector<double>& state, double r, std::size_t n, Scheme scheme){
        void (*step)(std::vector<double>&, double);
        switch (scheme){
            case Scheme::EXPLICIT:       step = time_step; break;
            case Scheme::BACKWARD_EULER: step = backward_euler; break;
            case Scheme::CRANK_NICOLSON: step = crank_nicolson; break;
            default: throw std::invalid_argument("Unknown scheme");
        }
        for (std::size_t i = 0; i < n; i++) {step(state, r);};
    }

}
