#include <vector>
#include <stdexcept>
#include <limits>
#include <iomanip>
#include "core/grid.hpp"
#include "writer/writer.hpp"

namespace heat{
    void write_to_stream(std::ostream& stream, double t, const Grid& grid, const std::vector<double>& calculated, const std::vector<double>& exact){
        
        if(exact.size() != calculated.size() || calculated.size() != grid.size()){
            throw std::invalid_argument("Size of grid, calculated vector and exact vector must be equal");
        }

        const int precision = std::numeric_limits<double>::max_digits10;
        stream << std::setprecision(precision);
        for(std::size_t i = 0; i < grid.size() ; i++){
            stream << t << "," << grid.x()[i] << "," << calculated[i] << "," << exact[i] << "\n";
        }
    }
    void write_header(std::ostream& stream){
        stream << "t,x,u_numeric,u_exact\n";
    }
}
