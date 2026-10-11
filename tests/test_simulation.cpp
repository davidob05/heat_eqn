#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include "driver/simulation.hpp"
#include "solver/time_step.hpp"
#include "core/initialise_rod.hpp"
#include "core/grid.hpp"
#include "verifier/verifier.hpp"
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <functional>
#include <ios>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>
#include <catch2/generators/catch_generators_range.hpp>

namespace {
    const std::string header = "t,x,u_numeric,u_exact\n";

    struct Block {
        std::vector<double> t;
        std::vector<double> x;
        std::vector<double> numeric;
        std::vector<double> exact;
    };

    // splits simulation output into blocks of n_points rows, skipping the header line
    std::vector<Block> parse_blocks(const std::string& text, std::size_t n_points){
        std::istringstream i_stream(text);
        std::string line;
        std::getline(i_stream,line,'\n');
        std::vector<Block> blocks;
        std::size_t row = 0;
        while(std::getline(i_stream,line,'\n')){
            if(row % n_points == 0){
                blocks.emplace_back();
            }
            std::istringstream i_stream_vars(line);
            std::string stored_vars;
            std::getline(i_stream_vars,stored_vars,',');
            blocks.back().t.push_back(std::stod(stored_vars));
            std::getline(i_stream_vars,stored_vars,',');
            blocks.back().x.push_back(std::stod(stored_vars));
            std::getline(i_stream_vars,stored_vars,',');
            blocks.back().numeric.push_back(std::stod(stored_vars));
            std::getline(i_stream_vars,stored_vars,',');
            blocks.back().exact.push_back(std::stod(stored_vars));
            row++;
        }
        return blocks;
    }

    double max_error(const Block& block){
        double max_err = 0.0;
        for(std::size_t i = 0 ; i < block.numeric.size() ; i++ ){
            max_err = std::max(max_err,std::abs(block.numeric[i] - block.exact[i]));
        }
        return max_err;
    }

    // N=10, T=0.0105, r_req=0.4 gives n=3, dt=0.0035, r=0.35
    heat::SimulationParams small_params(std::size_t snapshots){
        heat::SimulationParams params;
        params.n_intervals = 10;
        params.T = 0.0105;
        params.snapshots = snapshots;
        return params;
    }

    // N=10, T=0.027, r_req=0.4 gives n=7
    heat::SimulationParams seven_step_params(std::size_t snapshots){
        heat::SimulationParams params;
        params.n_intervals = 10;
        params.T = 0.027;
        params.snapshots = snapshots;
        return params;
    }
}

TEST_CASE("run_simulation throws if the output stream has failed"){
    heat::SimulationParams params;
    std::ostringstream stream;
    stream.setstate(std::ios::badbit);
    CHECK_THROWS_AS(heat::run_simulation(params,stream),std::runtime_error);
    CHECK(stream.str().empty());
}

TEST_CASE("run_simulation does not throw on a healthy stream"){
    heat::SimulationParams params;
    std::ostringstream stream;
    CHECK_NOTHROW(heat::run_simulation(params,stream));
    CHECK(!stream.str().empty());
}

TEST_CASE("Snapshots equal to n runs and writes S+1 blocks"){
    std::ostringstream stream;
    heat::SimulationResult result = heat::run_simulation(small_params(3),stream);
    CHECK(result.n_steps == 3);
    CHECK(parse_blocks(stream.str(),11).size() == 4);
}

TEST_CASE("More snapshots than steps throws and writes nothing"){
    std::ostringstream stream;
    CHECK_THROWS_AS(heat::run_simulation(small_params(4),stream),std::invalid_argument);
    CHECK(stream.str().empty());
}

TEST_CASE("Zero snapshots throws and writes nothing"){
    std::ostringstream stream;
    CHECK_THROWS_AS(heat::run_simulation(small_params(0),stream),std::invalid_argument);
    CHECK(stream.str().empty());
}

TEST_CASE("Invalid parameters from lower layers throw and write nothing"){
    heat::SimulationParams params;
    std::ostringstream stream;

    SECTION("mode 0"){
        params.mode = 0;
    }
    SECTION("zero intervals"){
        params.n_intervals = 0;
    }
    SECTION("non-positive length"){
        params.length = GENERATE(as<double>{},0.0,-1.0);
    }
    SECTION("non-positive T"){
        params.T = GENERATE(as<double>{},0.0,-0.1);
    }
    SECTION("negative alpha"){
        params.alpha = -1.0;
    }

    CHECK_THROWS_AS(heat::run_simulation(params,stream),std::invalid_argument);
    CHECK(stream.str().empty());
}

TEST_CASE("Header appears once and first"){
    heat::SimulationParams params;
    std::ostringstream stream;
    heat::run_simulation(params,stream);
    const std::string text = stream.str();
    CHECK(text.rfind(header,0) == 0);
    CHECK(text.find(header,1) == std::string::npos);
}

TEST_CASE("Output has the expected number of lines"){
    heat::SimulationParams params;
    std::ostringstream stream;
    heat::run_simulation(params,stream);
    const std::string text = stream.str();
    CHECK(std::count(text.begin(),text.end(),'\n') == 1 + 6*101);
}

TEST_CASE("Time column starts at 0, is constant in a block and increases across blocks"){
    heat::SimulationParams params;
    std::ostringstream stream;
    heat::run_simulation(params,stream);
    std::vector<Block> blocks = parse_blocks(stream.str(),101);
    REQUIRE(blocks.size() == 6);
    CHECK(blocks[0].t[0] == 0.0);
    for(std::size_t b = 0 ; b < blocks.size() ; b++ ){
        INFO("block " << b);
        for(std::size_t i = 0 ; i < blocks[b].t.size() ; i++ ){
            CHECK(blocks[b].t[i] == blocks[b].t[0]);
        }
        if(b > 0){
            CHECK(blocks[b].t[0] > blocks[b-1].t[0]);
        }
    }
}

TEST_CASE("First block is the initial sine with zero ends"){
    heat::SimulationParams params;
    std::ostringstream stream;
    heat::run_simulation(params,stream);
    std::vector<Block> blocks = parse_blocks(stream.str(),101);
    heat::Grid grid(params.n_intervals,params.length);
    std::function<double(double)> f = heat::generate_specific_sin(params.length,params.mode);
    const double eps = std::numeric_limits<double>::epsilon();
    CHECK(blocks[0].numeric.front() == 0.0);
    CHECK(blocks[0].numeric.back() == 0.0);
    for(std::size_t i = 1 ; i < grid.size() - 1 ; i++ ){
        INFO("row " << i);
        CHECK_THAT(blocks[0].numeric[i],Catch::Matchers::WithinAbs(f(grid.x()[i]),10*eps));
    }
}

TEST_CASE("Last block parses back to the returned result"){
    heat::SimulationParams params;
    std::ostringstream stream;
    heat::SimulationResult result = heat::run_simulation(params,stream);
    std::vector<Block> blocks = parse_blocks(stream.str(),101);
    REQUIRE(blocks.size() == 6);
    CHECK(blocks.back().numeric == result.calculated);
    CHECK(blocks.back().exact == result.exact);
    for(std::size_t i = 0 ; i < blocks.back().t.size() ; i++ ){
        CHECK(blocks.back().t[i] == result.final_time);
    }
}

TEST_CASE("Snapshot spacing is floor or ceil of n/S"){
    //n=7, S=3 gives chunks of 3, 2, 2
    std::ostringstream stream;
    heat::SimulationResult result = heat::run_simulation(seven_step_params(3),stream);
    REQUIRE(result.n_steps == 7);
    std::vector<Block> blocks = parse_blocks(stream.str(),11);
    REQUIRE(blocks.size() == 4);
    std::size_t total = 0;
    for(std::size_t b = 1 ; b < blocks.size() ; b++ ){
        INFO("block " << b);
        const double spacing = std::round((blocks[b].t[0] - blocks[b-1].t[0])/result.dt);
        CHECK((spacing == 2.0 || spacing == 3.0));
        total += static_cast<std::size_t>(spacing);
    }
    CHECK(total == 7);
}

TEST_CASE("Snapshot count does not change the final state"){
    heat::SimulationParams params;
    std::ostringstream stream_one;
    std::ostringstream stream_five;
    params.snapshots = 1;
    heat::SimulationResult result_one = heat::run_simulation(params,stream_one);
    params.snapshots = 5;
    heat::SimulationResult result_five = heat::run_simulation(params,stream_five);
    CHECK(result_one.calculated == result_five.calculated);
}

TEST_CASE("Snapshots equal to n gives one step per snapshot"){
    std::ostringstream stream;
    heat::SimulationResult result = heat::run_simulation(small_params(3),stream);
    std::vector<Block> blocks = parse_blocks(stream.str(),11);
    REQUIRE(blocks.size() == 4);
    for(std::size_t b = 1 ; b < blocks.size() ; b++ ){
        INFO("block " << b);
        CHECK(std::round((blocks[b].t[0] - blocks[b-1].t[0])/result.dt) == 1.0);
    }
}

TEST_CASE("Result step parameters match the step parameter calculator"){
    heat::SimulationParams params;
    std::ostringstream stream;
    heat::SimulationResult result = heat::run_simulation(params,stream);
    heat::Grid grid(params.n_intervals,params.length);
    auto [n,dt,r] = heat::calculate_step_parameters(grid.dx(),params.T,params.alpha,params.r_req);
    CHECK(result.n_steps == n);
    CHECK(result.n_steps == 2500);
    CHECK(result.dt == dt);
    CHECK(result.r_used == r);
}

TEST_CASE("Final time is T to within rounding"){
    heat::SimulationParams params;
    std::ostringstream stream;
    heat::SimulationResult result = heat::run_simulation(params,stream);
    CHECK_THAT(result.final_time,Catch::Matchers::WithinRel(params.T,10*std::numeric_limits<double>::epsilon()));
}

TEST_CASE("Result vectors have the right size and exact matches the verifier"){
    heat::SimulationParams params;
    std::ostringstream stream;
    heat::SimulationResult result = heat::run_simulation(params,stream);
    heat::Grid grid(params.n_intervals,params.length);
    CHECK(result.calculated.size() == params.n_intervals + 1);
    CHECK(result.exact == heat::exact_soln(grid,params.mode,result.final_time,params.alpha));
}

TEST_CASE("Decay against an exact solution through the driver"){
    //observed difference of 4.2e-5
    heat::SimulationParams params;
    std::ostringstream stream;
    heat::run_simulation(params,stream);
    std::vector<Block> blocks = parse_blocks(stream.str(),101);
    CHECK_THAT(max_error(blocks.back()),Catch::Matchers::WithinAbs(0.0,6e-5));
}

TEST_CASE("Block maximum never increases and never exceeds the initial maximum"){
    heat::SimulationParams params;
    std::ostringstream stream;
    heat::run_simulation(params,stream);
    std::vector<Block> blocks = parse_blocks(stream.str(),101);
    const double initial_max = *std::max_element(blocks[0].numeric.begin(),blocks[0].numeric.end());
    double prev_max = initial_max;
    for(std::size_t b = 1 ; b < blocks.size() ; b++ ){
        INFO("block " << b);
        const double block_max = *std::max_element(blocks[b].numeric.begin(),blocks[b].numeric.end());
        CHECK(block_max <= prev_max);
        CHECK(block_max <= initial_max);
        prev_max = block_max;
    }
}

TEST_CASE("Mode 2 decay against an exact solution through the driver"){
    heat::SimulationParams params;
    params.mode = 2;
    std::ostringstream stream;
    heat::run_simulation(params,stream);
    std::vector<Block> blocks = parse_blocks(stream.str(),101);
    CHECK_THAT(max_error(blocks.back()),Catch::Matchers::WithinAbs(0.0,5.3e-5));
    //observed difference of 3.5e-5
}

TEST_CASE("Default parameters give no warnings"){
    heat::SimulationParams params;
    std::ostringstream stream;
    heat::SimulationResult result = heat::run_simulation(params,stream);
    CHECK(result.warnings.empty());
}

TEST_CASE("Unstable r warning fires only above 0.5"){
    heat::SimulationParams params;
    std::ostringstream stream;

    params.r_req = 0.5;
    CHECK(heat::run_simulation(params,stream).warnings.empty());

    params.r_req = 0.51;
    heat::SimulationResult result = heat::run_simulation(params,stream);
    REQUIRE(result.warnings.size() == 1);
    CHECK(result.warnings[0].code == heat::WarningCode::UNSTABLE_R);
}

TEST_CASE("Mode too high warning fires only past the threshold"){
    heat::SimulationParams params;
    params.n_intervals = 100;
    params.T = 1e-4;
    params.snapshots = 1;
    std::ostringstream stream;

    params.mode = GENERATE(as<int>{},10,-10);
    CHECK(heat::run_simulation(params,stream).warnings.empty());

    params.mode = params.mode > 0 ? 11 : -11;
    heat::SimulationResult result = heat::run_simulation(params,stream);
    REQUIRE(result.warnings.size() == 1);
    CHECK(result.warnings[0].code == heat::WarningCode::MODE_TOO_HIGH_FOR_INTERVALS);
}
