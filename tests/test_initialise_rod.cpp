#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include "solver/initialise_rod.hpp"
#include "solver/grid.hpp"
#include <limits>
#include <stdexcept>
#include <functional>
#include <cmath>
#include <string>
#include <vector>
#include <catch2/generators/catch_generators_range.hpp>

namespace {

    struct TestFunction{
        std::string name;
        std::function<double(double, double)> f; // f(x, length)
    };

    std::vector<TestFunction> test_functions(){
        return {
            {"0",          [](double, double)     {return 0.0;}},
            {"sin(x)",     [](double x, double)   {return std::sin(x);}},
            {"x",          [](double x, double)   {return x;}},
            {"-1",         [](double, double)     {return -1.0;}},
            {"length - x", [](double x, double L) {return L - x;}}
        };
    }

}//namespace

TEST_CASE("Initial rod has N+1 points"){
    auto n_intervals = GENERATE(as<std::size_t>{},12,20,1000,1,49);
    auto length = GENERATE(as<double>{},0.7,1.0,100.58,0.1);
    auto test_fn = GENERATE(from_range(test_functions()));

    auto function = [&](double x) {return test_fn.f(x, length);};

    INFO("built from intervals = " << n_intervals << ", and length = " << length << ", and function = " << test_fn.name);
    CHECK(heat::initialise_rod(heat::Grid(n_intervals,length),function).size() == n_intervals + 1);

}

TEST_CASE("Initial rod has 0 at position 0"){
    auto n_intervals = GENERATE(as<std::size_t>{},12,20,1000,1,49);
    auto length = GENERATE(as<double>{},0.7,1.0,100.58,0.1);
    auto test_fn = GENERATE(from_range(test_functions()));

    auto function = [&](double x) {return test_fn.f(x, length);};

    INFO("built from intervals = " << n_intervals << ", and length = " << length << ", and function = " << test_fn.name);
    CHECK(heat::initialise_rod(heat::Grid(n_intervals,length),function)[0] == 0);

}

TEST_CASE("Initial rod has 0 at the end"){
    auto n_intervals = GENERATE(as<std::size_t>{},12,20,1000,1,49);
    auto length = GENERATE(as<double>{},0.7,1.0,100.58,0.1);
    auto test_fn = GENERATE(from_range(test_functions()));

    auto function = [&](double x) {return test_fn.f(x, length);};

    INFO("built from intervals = " << n_intervals << ", and length = " << length << ", and function = " << test_fn.name);
    CHECK(heat::initialise_rod(heat::Grid(n_intervals,length),function).back() == 0);

}

TEST_CASE("Initial rod has expected values for given function"){
    auto n_intervals = GENERATE(as<std::size_t>{},12,20,1000,1,49);
    auto length = GENERATE(as<double>{},0.7,1.0,100.58,0.1);
    auto test_fn = GENERATE(from_range(test_functions()));
    auto function = [&](double x) {return test_fn.f(x, length);};

    INFO("built from intervals = " << n_intervals << ", and length = " << length << ", and function = " << test_fn.name);
    heat::Grid grid = heat::Grid(n_intervals,length);
    std::vector<double> rod = heat::initialise_rod(grid,function);
    for(std::size_t i = 1 ; i < rod.size() -1 ; i++){
        CHECK(rod[i] == function(grid.x()[i]));
    }
}
