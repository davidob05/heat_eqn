#include "simulation.hpp"
#include "core/grid.hpp"
#include "solver/time_step.hpp"
#include "core/initialise_rod.hpp"
#include "verifier/verifier.hpp"
#include "writer/writer.hpp"
#include <tuple>
#include <stdexcept>
#include <cmath>
#include <limits>
#include <sstream>
#include <iomanip>
#include <string>
#include <vector>


namespace heat{
    SimulationResult run_simulation(const SimulationParams& params ,std::ostream& stream){
        
        SimulationResult sim_result;

        if(std::abs(static_cast<double>(params.mode))*MODE_N_INTERVALS_THRESHOLD > static_cast<double>(params.n_intervals)){
            sim_result.warnings.push_back(Warning(WarningCode::MODE_TOO_HIGH_FOR_INTERVALS, "mode " + std::to_string(params.mode) + " is not suitable for number of intervals " + std::to_string(params.n_intervals)));

        }

        if(std::exp(-params.alpha*pi*pi*params.mode*params.mode*params.T/(params.length*params.length)) < DECAY_THRESHOLD){
            sim_result.warnings.push_back(Warning(WarningCode::EXTREME_DECAY, "State decays to near 0, error margin may be misleading at the latter snapshots"));
        }
        
        Grid grid(params.n_intervals,params.length);
        auto [n,dt,r] = calculate_step_parameters(grid.dx(),params.T,params.alpha,params.r_req);
        
        

        if(params.snapshots>n){
            throw std::invalid_argument("There must be at least as many timesteps as snapshots.");
        }


        if(r>0.5*(1+ 10*std::numeric_limits<double>::epsilon())){
            std::ostringstream r_stream;
            r_stream << std::setprecision(std::numeric_limits<double>::max_digits10) << r;
            const std::string r_str = r_stream.str();
            sim_result.warnings.push_back(Warning(WarningCode::UNSTABLE_R, "value " + r_str + " is not suitable, stable r should be between 0.0 and 0.5"));
        }

        if( params.snapshots == 0 ){
            throw std::invalid_argument("Number of snapshots must not be 0");
        }

        auto initialiser_function = generate_specific_sin(params.length, params.mode);
        auto [left_0, right_0 ] = check_fn_0_endpoints(grid,initialiser_function, params.mode);
        if(left_0){
            sim_result.warnings.push_back(Warning(WarningCode::LEFT_ENDPOINT, "left endpoint of initialiser function not close enough to 0"));
        }

        if(right_0){
            sim_result.warnings.push_back(Warning(WarningCode::RIGHT_ENDPOINT, "right endpoint of initialiser function not 0"));
        }

        std::vector<double> curr_state = initialise_rod(grid,initialiser_function);
        std::vector<double> exact = exact_soln(grid, params.mode, 0.0, params.alpha);

        write_header(stream);
        write_to_stream(stream, 0, grid, curr_state, exact);

        std::size_t snapshot_remainder = n%params.snapshots;
        std::size_t chunk_size = n/params.snapshots;
        std::size_t runs_done = 0;

        for(std::size_t i = 0 ; i < params.snapshots ; i++ ){
            std::size_t n_runs = chunk_size;

            if( snapshot_remainder > 0 ){
                n_runs = n_runs + 1;
                snapshot_remainder--;
            }

            step_loop(curr_state,r,n_runs);
            runs_done += n_runs;

            double snapshot_time = static_cast<double>(runs_done)*dt;

            exact = exact_soln(grid, params.mode, snapshot_time, params.alpha);

            write_to_stream(stream, snapshot_time, grid, curr_state, exact);
        }

        stream.flush();
        if(stream.fail()){
            throw std::runtime_error("Failed to write simulation output to the stream.");
        }

        sim_result.n_steps = runs_done;
        sim_result.dt = dt;
        sim_result.r_used = r;
        sim_result.final_time = static_cast<double>(runs_done)*dt;
        sim_result.calculated = std::move(curr_state);
        sim_result.exact = std::move(exact);
        return sim_result;
    }
}
