#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include "driver/cli.hpp"
#include <cstddef>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>
#include <catch2/generators/catch_generators_range.hpp>

TEST_CASE("No arguments gives the defaults"){
    heat::CliOptions options = heat::parse_args({});
    heat::SimulationParams defaults;
    CHECK(options.params.n_intervals == defaults.n_intervals);
    CHECK(options.params.length == defaults.length);
    CHECK(options.params.alpha == defaults.alpha);
    CHECK(options.params.T == defaults.T);
    CHECK(options.params.r_req == defaults.r_req);
    CHECK(options.params.mode == defaults.mode);
    CHECK(options.params.snapshots == defaults.snapshots);
    CHECK(options.output_path == "results/output.csv");
    CHECK(!options.show_help);
    CHECK(!options.plot);
}

TEST_CASE("Every flag fills in its field"){
    heat::CliOptions options = heat::parse_args({"--n-intervals","50","--length","2.5","--alpha","0.5","--T","0.05","--r-req","0.3","--mode","-2","--snapshots","4","--output","out.csv","--plot"});
    CHECK(options.params.n_intervals == 50);
    CHECK(options.params.length == 2.5);
    CHECK(options.params.alpha == 0.5);
    CHECK(options.params.T == 0.05);
    CHECK(options.params.r_req == 0.3);
    CHECK(options.params.mode == -2);
    CHECK(options.params.snapshots == 4);
    CHECK(options.output_path == "out.csv");
    CHECK(options.plot);
}

TEST_CASE("Space separated and equals separated values are equivalent"){
    heat::CliOptions spaced = heat::parse_args({"--T","0.05","--mode","3","--output","a.csv"});
    heat::CliOptions equals = heat::parse_args({"--T=0.05","--mode=3","--output=a.csv"});
    CHECK(spaced.params.T == equals.params.T);
    CHECK(spaced.params.mode == equals.params.mode);
    CHECK(spaced.output_path == equals.output_path);
}

TEST_CASE("Help flags set show_help"){
    auto flag = GENERATE(as<std::string>{},"--help","-h");
    CHECK(heat::parse_args({flag}).show_help);
}

TEST_CASE("Plot takes no value"){
    CHECK_THROWS_AS(heat::parse_args({"--plot=1"}),std::invalid_argument);
}

TEST_CASE("Malformed numbers are rejected"){
    CHECK_THROWS_AS(heat::parse_args({"--T","1.0abc"}),std::invalid_argument);
    CHECK_THROWS_AS(heat::parse_args({"--T","abc"}),std::invalid_argument);
    CHECK_THROWS_AS(heat::parse_args({"--T",""}),std::invalid_argument);
    CHECK_THROWS_AS(heat::parse_args({"--mode","1.5"}),std::invalid_argument);
    CHECK_THROWS_AS(heat::parse_args({"--snapshots","3x"}),std::invalid_argument);
}

TEST_CASE("Negative sizes are rejected"){
    CHECK_THROWS_AS(heat::parse_args({"--n-intervals","-5"}),std::invalid_argument);
    CHECK_THROWS_AS(heat::parse_args({"--snapshots","-1"}),std::invalid_argument);
}

TEST_CASE("Mode outside the int range is rejected rather than wrapped"){
    CHECK_THROWS_AS(heat::parse_args({"--mode","4294967297"}),std::invalid_argument);
    CHECK_THROWS_AS(heat::parse_args({"--mode","-4294967297"}),std::invalid_argument);
    CHECK_THROWS_AS(heat::parse_args({"--mode","2147483648"}),std::invalid_argument);
    CHECK(heat::parse_args({"--mode","2147483647"}).params.mode == 2147483647);
}

TEST_CASE("Intervals above the cap are rejected"){
    CHECK_THROWS_AS(heat::parse_args({"--n-intervals","99999999999"}),std::invalid_argument);
    CHECK(heat::parse_args({"--n-intervals",std::to_string(heat::MAX_CLI_INTERVALS)}).params.n_intervals == heat::MAX_CLI_INTERVALS);
    CHECK_THROWS_AS(heat::parse_args({"--n-intervals",std::to_string(heat::MAX_CLI_INTERVALS + 1)}),std::invalid_argument);
}

TEST_CASE("Unknown flags are reported as unknown, even at the end of the line"){
    for(const std::vector<std::string>& args : std::vector<std::vector<std::string>>{{"--bogus"},{"--bogus","1"},{"--T","0.1","--bogus"}}){
        try{
            heat::parse_args(args);
            FAIL("expected an exception");
        } catch(const std::invalid_argument& e){
            CHECK(std::string(e.what()).find("unknown option") != std::string::npos);
        }
    }
}

TEST_CASE("A value flag with no value is reported as missing"){
    try{
        heat::parse_args({"--T"});
        FAIL("expected an exception");
    } catch(const std::invalid_argument& e){
        CHECK(std::string(e.what()).find("missing value") != std::string::npos);
    }
}

TEST_CASE("Usage text mentions every flag"){
    std::ostringstream stream;
    heat::print_usage(stream,"Main");
    for(const char* flag : {"--n-intervals","--length","--alpha","--T","--r-req","--mode","--snapshots","--output","--plot","--help"}){
        INFO(flag);
        CHECK(stream.str().find(flag) != std::string::npos);
    }
}
