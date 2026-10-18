#include "verifier.hpp"
#include "core/initialise_rod.hpp"
#include <cmath>
#include <stdexcept>
#include <vector>

namespace heat{
    std::function<double(double)> generate_specific_sin(double length, int mode){
        
        if( mode == 0 ) throw std::invalid_argument("Mode for sin(mode*pi*x/L) is 0");
        if( not (length > 0) || std::isinf(length)) throw std::invalid_argument("Mode must be defined and not infinite");

        return [length,mode](double x) {
            return std::sin((mode*pi*x)/length);
        };
    }

    std::vector<double> exact_soln(const Grid& grid, int mode, double t, double alpha){
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

    double max_error(const std::vector<double>& calculated, const std::vector<double>& exact){
        double max_found_error = 0.0;
        if( calculated.size() != exact.size() ){
            throw std::invalid_argument("Exact vector and calculated vector must have equal size");
        }

        if( calculated.size() == 0 ){
            throw std::invalid_argument("0 size input vector");
        }

        for( std::size_t j = 0 ; j < exact.size() ; j ++){
            double contender = std::abs(exact[j] - calculated[j]);
            if( ! std::isfinite(contender)){
                throw std::invalid_argument("Non-finite error, check parameters passed");
            }
            if( max_found_error < contender){
                max_found_error = contender;
            }
        }

        return max_found_error;
    }

    double l2_error(const std::vector<double>& calculated, const std::vector<double>& exact, double dx){
        if( calculated.size() != exact.size() ){
            throw std::invalid_argument("Inputted vectors must be of equal size");
        }

        if( calculated.size() == 0 ){
            throw std::invalid_argument("Size of input vectors must be greater than 0");
        }

        if( !std::isfinite(dx) || dx <= 0.0){
            throw std::invalid_argument("dx must be positive and finite");
        }

        double sum_sq = 0.0;

        for (std::size_t i = 0 ; i < calculated.size() ; i++ ){
            sum_sq = sum_sq + (calculated[i] - exact[i])*(calculated[i] - exact[i]);
        }

        double l2_norm = std::sqrt(sum_sq*dx);
        
        if (!std::isfinite(l2_norm)){
            throw std::invalid_argument("Sum of squares multiplied with dx of difference gives infinite value, vectors too far apart and/or dx too large");
        }

        return l2_norm;
    }
}
