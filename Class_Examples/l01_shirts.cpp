#include <array>
#include <cstdlib>
#include <exception>
#include <iostream>
#include <string>
#include <vector>

#include "cplex_utils.hpp"

// A group of kids wants to earn as much as possible by selling decorated
// t-shirts and bags to their friends. There are 10 cotton t-shirts and 15
// canvas bags available and, for decoration, 32 designed DTF stickers and 40
// red trims. Each t-shirt gets 6 stickers and 2 trims, and each bag gets
// 3 stickers and 5 trims. There are also 15 buttons available, and each bag
// uses two of them for its closure. Moreover, 22 labels have been prepared,
// to be applied one on each t-shirt and two on each bag. Considering that
// each decorated t-shirt is sold for 24 Euros and each bag for 16 Euros, and
// that the friends will buy all t-shirts and bags, determine the production
// that maximizes the revenue.

// == Variables
// x_s, x_b = # t-shirts/bags produced

// == Model
// max 24 * x_s + 16 * x_b
// s.t.
//          x_s            <= 10 # t-shirts
//                     x_b <= 15 # bags
//      6 * x_s +  3 * x_b <= 32 # stickers
//      2 * x_s +  5 * x_b <= 40 # trims
//                 2 * x_b <= 15 # buttons
//      1 * x_s +  2 * x_b <= 22 # labels
// x_s, x_b in Z_+

namespace {
auto write_lp(CPXENVptr env, CPXLPptr lp) -> void {
    // Adding the variables
    CPLEX_add_variable(env, lp, 24, 0, CPX_INFBOUND, 'I', std::string("x_s").data());
    CPLEX_add_variable(env, lp, 16, 0, CPX_INFBOUND, 'I', std::string("x_b").data());
    auto [x_s, x_b] = std::pair{0, 1};

    // Adding the constraints
    CPLEX_add_constraint(env, lp, 10, 'L', {x_s}, {1}, std::string("t-shirts").data());
    CPLEX_add_constraint(env, lp, 15, 'L', {x_b}, {1}, std::string("bags").data());
    CPLEX_add_constraint(env, lp, 32, 'L', {x_s, x_b}, {6, 3}, std::string("stickers").data());
    CPLEX_add_constraint(env, lp, 40, 'L', {x_s, x_b}, {2, 5}, std::string("trims").data());
    CPLEX_add_constraint(env, lp, 15, 'L', {x_b}, {2}, std::string("buttons").data());
    CPLEX_add_constraint(env, lp, 22, 'L', {x_s, x_b}, {1, 2}, std::string("labels").data());

    // Objective sense (maximize)
    CPLEX_call(CPXchgobjsen, env, lp, CPX_MAX);

    // Write .lp to check if the model is correct
    CPLEX_call(CPXwriteprob, env, lp, "test.lp", nullptr);
}
}  // namespace

auto main() -> int {
    auto exit_code = EXIT_SUCCESS;
    auto [env, lp] = CPLEX_open("shirts");

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