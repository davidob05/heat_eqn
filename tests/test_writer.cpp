#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include "verifier/verifier.hpp"
#include "core/initialise_rod.hpp"
#include "writer/writer.hpp"
#include "core/grid.hpp"
#include <limits>
#include <functional>
#include <cmath>
#include <string>
#include <vector>
#include <catch2/generators/catch_generators_range.hpp>
#include <stdexcept>
#include <sstream>
#include <iostream>

TEST_CASE("Header function writes correct header"){
    std::ostringstream stream;
    heat::write_header(stream);
    CHECK(stream.str() == "t,x,u_numeric,u_exact\n");
}

TEST_CASE("Text on small grid"){
    std::ostringstream stream;
    heat::Grid grid(4,1.0);
    heat::write_to_stream(stream, 0.5, grid, {0,1,2,3,4}, {10,20,30,40,50});
    CHECK(stream.str() == "0.5,0,0,10\n0.5,0.25,1,20\n0.5,0.5,2,30\n0.5,0.75,3,40\n0.5,1,4,50\n");
}

TEST_CASE("Check header call and two write calls work"){
    std::ostringstream stream;
    heat::Grid grid(4,1.0);
    heat::write_header(stream);
    heat::write_to_stream(stream, 0.5, grid, {0,1,2,3,4}, {10,20,30,40,50});
    heat::write_to_stream(stream, 0.25, grid, {0,0.5,1,1.5,2}, {5,6,7,8,9});
    CHECK(stream.str()=="t,x,u_numeric,u_exact\n0.5,0,0,10\n0.5,0.25,1,20\n0.5,0.5,2,30\n0.5,0.75,3,40\n0.5,1,4,50\n0.25,0,0,5\n0.25,0.25,0.5,6\n0.25,0.5,1,7\n0.25,0.75,1.5,8\n0.25,1,2,9\n");
}

TEST_CASE("Written values parse back to exactly the same doubles"){
    std::ostringstream stream;
    heat::Grid grid(3,1.0);
    double t = 0.1;
    std::vector calculated = {1.0/3, -0.1, 1e-17, 1e300};
    std::vector exact = {2.0/3, 0.7, 1e-300, -5.5e-8};
    heat::write_to_stream(stream, t, grid, calculated, exact);
    std::istringstream i_stream(stream.str());
    std::string stored_line;
    std::string stored_vars;
    for(std::size_t i = 0 ; i<exact.size() ; i++){
        INFO("row " << i);
        std::getline(i_stream,stored_line,'\n');
        std::istringstream i_stream_vars(stored_line);
        std::getline(i_stream_vars, stored_vars,',');
        CHECK(std::stod(stored_vars) == t);
        std::getline(i_stream_vars, stored_vars,',');
        CHECK(std::stod(stored_vars) == grid.x()[i]);
        std::getline(i_stream_vars, stored_vars,',');
        CHECK(std::stod(stored_vars) == calculated[i]);
        std::getline(i_stream_vars, stored_vars,',');
        CHECK(std::stod(stored_vars) == exact[i]);
    }
    std::getline(i_stream,stored_line,'\n');
    CHECK(i_stream.fail());
}


TEST_CASE("Check stdexcept throws are correct"){
    std::ostringstream stream;
    heat::Grid grid(4,1.0);
    CHECK_THROWS_AS(heat::write_to_stream(stream, 0.5, grid, {0,1,2,3,4,5}, {10,20,30,40,50}), std::invalid_argument);
    CHECK_THROWS_AS(heat::write_to_stream(stream, 0.5, grid, {0,1,2,3,4}, {10,20,30,40,50,60}), std::invalid_argument);
    CHECK_THROWS_AS(heat::write_to_stream(stream, 0.5, grid, {0,1,2,3}, {10,20,30,40}), std::invalid_argument);
}

TEST_CASE("Check invalid calls do not write to stream"){
    std::ostringstream stream;
    heat::Grid grid(4,1.0);
    CHECK_THROWS_AS(heat::write_to_stream(stream, 0.5, grid, {0,1,2,3,4,5}, {10,20,30,40,50}), std::invalid_argument);
    CHECK(stream.str()=="");
}
