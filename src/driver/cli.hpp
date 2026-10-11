#pragma once
#include "driver/simulation.hpp"
#include <cstddef>
#include <ostream>
#include <string>
#include <vector>

namespace heat{
    // sanity cap so absurd --n-intervals values fail with a clear message instead of bad_alloc
    inline constexpr std::size_t MAX_CLI_INTERVALS = 10000000;

    struct CliOptions {
        SimulationParams params;
        std::string output_path = "results/output.csv";
        bool show_help = false;
        bool plot = false;
    };

    void print_usage(std::ostream& stream, const std::string& program);
    // Throws std::invalid_argument naming the offending flag if the arguments are malformed.
    CliOptions parse_args(const std::vector<std::string>& args);
}
