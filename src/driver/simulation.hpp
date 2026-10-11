#pragma once
#include <ostream>
#include <cstddef>
#include <vector>


namespace heat{

    inline constexpr double MODE_N_INTERVALS_THRESHOLD = 10.0;
    inline constexpr double DECAY_THRESHOLD = 1e-10;

    struct SimulationParams{
        std::size_t n_intervals = 100;
        double length = 1.0;
        double alpha = 1.0;
        double T = 0.1;
        double r_req = 0.4;
        int mode = 1;
        std::size_t snapshots = 5;
    };

    enum class WarningCode {
        UNSTABLE_R,
        LEFT_ENDPOINT,
        RIGHT_ENDPOINT,
        MODE_TOO_HIGH_FOR_INTERVALS,
        EXTREME_DECAY
    };

    struct Warning{
        WarningCode code;
        std::string message;

        Warning(WarningCode code, std::string message)
            : code(code), message(std::move(message)) {}
    };

    struct SimulationResult{
        std::size_t n_steps = 0;
        double dt = 0.0;
        double r_used = 0.0;
        double final_time = 0.0;
        std::vector<double> calculated;
        std::vector<double> exact;
        std::vector<Warning> warnings;
    };

    // Throws std::runtime_error if the stream is in a failed state after the final flush.
    SimulationResult run_simulation(const SimulationParams& params ,std::ostream& stream);
}
