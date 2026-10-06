#include <cstdlib>
#include <exception>
#include <iostream>
#include <numeric>
#include <string>
#include <tuple>
#include <vector>

#include "cplex_utils.hpp"

// I   : resources
// J   : products
// D_i : availability of resource i in I
// P_j : unit profit for product j in J
// Q_ij: amount of resource i in I required for each unit of product j in J

// == Variables
// x_j : amount of product j in J

// == Model
// max sum_{j in J} P_j x_j
// s.t.
// sum_{j in J} Q_ij x_j <= D_i     forall i in I
// x_j in R_+/Z_+/{0,1} (depends on the type of product)       forall j in J

namespace {
// Data (with the perfumes example)
constexpr int I = 3;                     // rose, lily and violet
constexpr int J = 2;                     // perfume 1 and perfume 2
const std::vector<double> D{27, 21, 9};  // D[i] = availability of resource i in I
const std::vector<double> P{130, 100};   // P[j] = profit of product j in J
const std::vector<std::vector<double>> Q{
    {1.5, 1},
    {1, 1},
    {0.3, 0.5},
};  // Q[i][j] = amount of resource i in I required for each unit of product j in J

auto CPLEX_write_lp(CPXENVptr env, CPXLPptr lp) -> void {
    // Adding the variables (x_j has index j)
    CPLEX_add_variables(env, lp, J, P, {}, {}, {}, make_names("x", J));

    // Adding the constraints (one per resource i)
    // Every constraint involves all the variables x_0, ..., x_{J-1}
    std::vector<int> row(J);
    std::ranges::iota(row, 0);
    const std::vector<std::vector<int>> indices(I, row);
    CPLEX_add_constraints(env, lp, I, D, std::vector<char>(I, 'L'), indices, Q, make_names("resource", I));

    // Objective sense (maximize)
    CPLEX_call(CPXchgobjsen, env, lp, CPX_MAX);

    // Write .lp to check if the model is correct
    CPLEX_call(CPXwriteprob, env, lp, "optimal_production.lp", nullptr);
}
}  // namespace

auto main() -> int {
    auto exit_code = EXIT_SUCCESS;
    CPXENVptr env{nullptr};
    CPXLPptr lp{nullptr};

    try {
        std::tie(env, lp) = CPLEX_open("production_mix");
        CPLEX_write_lp(env, lp);

        // This has no integer/binary variables, so we can solve it with Linear Programming (LP) and not Mixed Integer Programming (MIP)
        CPLEX_call(CPXlpopt, env, lp);

        CPLEX_print_solution(env, lp);
    } catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
        exit_code = EXIT_FAILURE;
    }

    CPLEX_close(env, lp);
    return exit_code;
}
