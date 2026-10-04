#include "verifier.hpp"
#include "core/initialise_rod.hpp"
#include <cmath>
#include <stdexcept>
#include <vector>

namespace heat{
    std::function<double(double)> generate_specific_sin(double length, int mode){
        
        if( mode == 0 ) throw std::invalid_argument("Mode for sin(mode*pi*x/L) is 0");
        if( not (length > 0) || std::isinf(length)) throw std::invalid_argument("Mode must be positive, defined and not infinite");

        return [length,mode](double x) {
            return std::sin((mode*pi*x)/length);
        };
    }

    std::vector<double> compute_end(const Grid& grid, int mode, double t, double alpha){
        double L = grid.length();
        if( not (alpha >= 0) || std::isinf(alpha)) throw std::invalid_argument("Alpha must be 0, positive, defined and not infinite");
        if( not (t >= 0) || std::isinf(t)) throw std::invalid_argument("Time must be 0, positive, defined and not infinite");

        std::function<double(double)> f = generate_specific_sin(L,mode);
        std::vector<double> solution = initialise_rod(grid,f);
        double decay_factor = std::exp(-alpha*static_cast<double>(mode)*static_cast<double>(mode)*pi*pi*t/(L*L));
        for(std::size_t i = 1 ; i < solution.size() - 1 ; i++){
            solution[i] = decay_factor*solution[i];
        }
        return solution;



        
    }
}
