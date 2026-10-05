#include <cstdlib>
#include <exception>
#include <iostream>
#include <string>
#include <vector>

#include "cplex_utils.hpp"

// A farmer owns 11 hectares of land where he can grow potatoes or
// tomatoes. Beyond the land, the available resources are: 70 kg of tomato
// seeds, 18 tons of potato tubers, 145 tons of fertilizer. The farmer knows
// that all his production can be sold with a profit of 6000 Euros per hectare
// of tomatoes and 7000 Euros per hectare of potatoes. Each hectare of tomatoes
// needs 7 kg seeds and 10 tons fertilizer. Each hectare of potatoes needs
// 3 tons tubers and 20 tons fertilizer. How does the farmer divide his land
// in order to gain as much as possible from the available resources?

// == Variables
// x_t, x_p = # hectares of tomatoes/potatoes

// == Model
// max 6000 * x_t + 7000 * x_p
// s.t.
//        1 * x_t +    1 * x_p <=  11 # land
//        7 * x_t              <=  70 # tomato seeds
//                     3 * x_p <=  18 # potato tubers
//       10 * x_t +   20 * x_p <= 145 # fertilizer
// x_t, x_p in R_+

namespace {
auto write_lp(CPXENVptr env, CPXLPptr lp) -> void {
    // Adding the variables
    CPLEX_add_variable(env, lp, 6000, 0, CPX_INFBOUND, 'C', std::string("x_t").data());
    CPLEX_add_variable(env, lp, 7000, 0, CPX_INFBOUND, 'C', std::string("x_p").data());
    auto [x_t, x_p] = std::pair{0, 1};

    // Adding the constraints
    CPLEX_add_constraint(env, lp, 11, 'L', {x_t, x_p}, {1, 1}, std::string("land").data());
    CPLEX_add_constraint(env, lp, 70, 'L', {x_t}, {7}, std::string("tomato seeds").data());
    CPLEX_add_constraint(env, lp, 18, 'L', {x_p}, {3}, std::string("potato tubers").data());
    CPLEX_add_constraint(env, lp, 145, 'L', {x_t, x_p}, {10, 20}, std::string("fertilizer").data());

    // Objective sense (maximize)
    CPLEX_call(CPXchgobjsen, env, lp, CPX_MAX);

    // Write .lp to check if the model is correct
    CPLEX_call(CPXwriteprob, env, lp, "test.lp", nullptr);
}
}  // namespace

auto main() -> int {
    auto exit_code = EXIT_SUCCESS;
    auto [env, lp] = CPLEX_open("farmer");

    try {
        write_lp(env, lp);

        CPLEX_call(CPXmipopt, env, lp);

        print_solution(env, lp);
    } catch (const std::exception& e) {
        std::cerr << e.what();
        exit_code = EXIT_FAILURE;
    }

    CPLEX_close(env, lp);
    return exit_code;
}