#include "verifier/verifier.hpp"
#include "solver/implicit/backward_euler.hpp"
#include "solver/implicit/crank_nicolson.hpp"
#include "solver/explicit/time_step.hpp"
#include "solver/scheme.hpp"
#include "core/grid.hpp"
#include "core/initialise_rod.hpp"
#include <vector>
#include <chrono>
#include <cstddef>
#include <iostream>
#include <algorithm>

double median (std::vector<double>& vec){
    std::sort(vec.begin(),vec.end());
    if (vec.size() % 2 == 0){
        return (vec[vec.size()/2] + vec[vec.size()/2 - 1])/2.0;
    }

    else {
        return vec[vec.size()/2];
    }
}

int main (){
    double length = 1.0;
    double alpha = 1.0;
    double T = 0.1;
    int mode = 1;
    double r_explicit = 0.5;
    double r_euler = 21.8696;
    double r_crank = 7340;
    std::size_t N = 2000;
    std::size_t runs = 5;

    heat::Grid grid(N,length);
    std::vector<double> rod = heat::initialise_rod(grid,heat::generate_specific_sin(length,mode));
    std::vector<double> rod_explicit = rod;
    std::vector<double> rod_euler = rod;
    std::vector<double> rod_crank = rod;
    std::vector<double> warmup = rod;

    std::vector<double> explicit_times(runs,0.0);
    std::vector<double> euler_times(runs,0.0);
    std::vector<double> crank_times(runs,0.0);
    std::vector<double> explicit_errors(runs,0.0);
    std::vector<double> euler_errors(runs,0.0);
    std::vector<double> crank_errors(runs,0.0);

    auto [n_explicit,dt_explicit,r_explicit_used] = heat::calculate_step_parameters(grid.dx(),T,alpha,r_explicit);
    auto [n_euler,dt_euler,r_euler_used] = heat::calculate_step_parameters(grid.dx(),T,alpha,r_euler);
    auto [n_crank,dt_crank,r_crank_used] = heat::calculate_step_parameters(grid.dx(),T,alpha,r_crank);

    std::vector<double> exact_solution = heat::exact_soln(grid,mode,T,alpha);

    heat::advance_state(warmup,r_explicit_used,n_explicit,heat::Scheme::EXPLICIT);


    for ( std::size_t i = 0 ; i < runs ; i++ ){
        auto start_explicit = std::chrono::steady_clock::now();
        heat::advance_state(rod_explicit,r_explicit_used,n_explicit,heat::Scheme::EXPLICIT);
        auto end_explicit = std::chrono::steady_clock::now();

        explicit_times[i] = std::chrono::duration<double>(end_explicit-start_explicit).count();
        explicit_errors[i] = heat::max_error(rod_explicit,exact_solution);
        rod_explicit = rod;
    }

    warmup = rod;
    heat::advance_state(warmup,r_euler_used,n_euler,heat::Scheme::BACKWARD_EULER);
    
    for ( std::size_t i = 0 ; i < runs ; i++ ){

        auto start_euler = std::chrono::steady_clock::now();
        heat::advance_state(rod_euler,r_euler_used,n_euler,heat::Scheme::BACKWARD_EULER);
        auto end_euler = std::chrono::steady_clock::now();

        euler_times[i] = std::chrono::duration<double>(end_euler-start_euler).count();
        euler_errors[i] = heat::max_error(rod_euler,exact_solution);
        rod_euler = rod;
    }

    warmup = rod;
    heat::advance_state(warmup,r_crank_used,n_crank,heat::Scheme::CRANK_NICOLSON);

    for ( std::size_t i = 0 ; i < runs ; i++ ){

        auto start_crank = std::chrono::steady_clock::now();
        heat::advance_state(rod_crank,r_crank_used,n_crank,heat::Scheme::CRANK_NICOLSON);
        auto end_crank = std::chrono::steady_clock::now();

        crank_times[i] = std::chrono::duration<double>(end_crank-start_crank).count();
        crank_errors[i] = heat::max_error(rod_crank,exact_solution);
        rod_crank = rod;
    }

    

    std::cout << "\nExplicit Scheme \n" << "\nNumber of runs: " << runs << "\nNumber of steps per run: " << n_explicit << "\nChange in time between steps: " << dt_explicit << "\nMedian error: " << median(explicit_errors) << "\nMedian Runtime: " << median(explicit_times) << "s"<< "\nMinimum: " << explicit_times[0] << "s\nMaximum: " << explicit_times.back() <<"s\n\n\n";
    std::cout << "Backward Euler Scheme \n" << "\nNumber of runs: " << runs << "\n Number of steps per run: " << n_euler << "\nChange in time between steps: " << dt_euler << "\nMedian error: " << median(euler_errors) << "\nMedian Runtime: " << median(euler_times) << "s"<< "\nMinimum: " << euler_times[0] << "s\nMaximum: " << euler_times.back() <<"s\n\n\n";
    std::cout << "Crank-Nicolson Scheme \n" << "\nNumber of runs: " << runs << "\n Number of steps per run: " << n_crank << "\nChange in time between steps: " << dt_crank << "\nMedian error: " << median(crank_errors) << "\nMedian Runtime: " << median(crank_times) << "s"<< "\nMinimum: " << crank_times[0] << "s\nMaximum: " << crank_times.back() <<"s\n\n\n";
}

