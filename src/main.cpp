#include "driver/cli.hpp"
#include "driver/simulation.hpp"
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

int main(int argc, char* argv[]){
    const std::vector<std::string> args(argv + 1, argv + argc);

    heat::CliOptions options;
    try{
        options = heat::parse_args(args);
    } catch(const std::invalid_argument& e){
        std::cerr << "Error: " << e.what() << "\n";
        heat::print_usage(std::cerr, argv[0]);
        return 2;
    }

    if(options.show_help){
        heat::print_usage(std::cout, argv[0]);
        return 0;
    }

    std::ofstream file(options.output_path);
    if(!file){
        std::cerr << "Error: could not open '" << options.output_path << "' for writing\n";
        return 1;
    }

    try{
        const heat::SimulationResult result = heat::run_simulation(options.params, file);
        for(const heat::Warning& warning : result.warnings){
            std::cerr << "Warning: " << warning.message << "\n";
        }
    } catch(const std::exception& e){
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    if(options.plot){
        std::cerr << "Note: --plot is not implemented yet, no plot produced\n";
    }

    // the ofstream destructor swallows close errors, so close explicitly and check
    file.close();
    if(file.fail()){
        std::cerr << "Error: failed to finish writing '" << options.output_path << "'\n";
        return 1;
    }
    return 0;
}
