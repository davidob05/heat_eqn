#include "driver/cli.hpp"
#include "driver/simulation.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <system_error>
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

    // write to a temporary file next to the target and rename over it only on success,
    // so a failed run never destroys the previous output
    const std::filesystem::path target(options.output_path);
    const std::filesystem::path temp = target.string() + ".tmp";

    std::ofstream file(temp);
    if(!file){
        std::cerr << "Error: could not open '" << temp.string() << "' for writing\n";
        return 1;
    }

    try{
        const heat::SimulationResult result = heat::run_simulation(options.params, file);
        for(const heat::Warning& warning : result.warnings){
            std::cerr << "Warning: " << warning.message << "\n";
        }
    } catch(const std::exception& e){
        std::cerr << "Error: " << e.what() << "\n";
        file.close();
        std::error_code ignored;
        std::filesystem::remove(temp, ignored);
        return 1;
    }

    // the ofstream destructor swallows close errors, so close explicitly and check
    file.close();
    std::error_code ec;
    if(file.fail()){
        std::cerr << "Error: failed to finish writing '" << temp.string() << "'\n";
        std::filesystem::remove(temp, ec);
        return 1;
    }

    std::filesystem::rename(temp, target, ec);
    if(ec){
        std::cerr << "Error: could not move output into place at '" << options.output_path << "': " << ec.message() << "\n";
        std::filesystem::remove(temp, ec);
        return 1;
    }
    return 0;
}
