#include <cstdlib>
#include <exception>
#include <iostream>
#include <string>
#include <tuple>
#include <vector>

#include "cplex_utils.hpp"

// I   : resources
// J   : requests
// C_i : unit cost of resource i in I
// R_j : requested amount of j in J
// A_ij: amount of request j in J satisfied by one unit of resource i in I

// == Variables
// x_i : amount of resource i in I

// == Model
// min sum_{i in I} C_i x_i
// s.t.
// sum_{i in I} A_ij x_i >= R_j     forall j in J
// x_i in R_+/Z_+/{0,1} (depends on the type of resource)       forall i in I

namespace {
// Data (with the diet example)
constexpr int I = 3;                      // vegetables, meat and fruit
constexpr int J = 3;                      // proteins, iron and calcium
const std::vector<double> C{4, 10, 7};    // C[i] = unit cost of resource i in I
const std::vector<double> R{20, 30, 10};  // R[j] = requested amount of j in J
const std::vector<std::vector<double>> A{
    {5, 6, 5},
    {15, 10, 3},
    {4, 5, 12},
};  // A[i][j] = amount of request j in J satisfied by one unit of resource i in I

auto CPLEX_write_lp(CPXENVptr env, CPXLPptr lp) -> void {
    // Adding the variables (x_i has index i)
    CPLEX_add_variables(env, lp, I, C, {}, {}, {}, make_names("x", I));

    // Adding the constraints (one per request j)
    std::vector<std::vector<int>> indices(J);
    std::vector<std::vector<double>> coeffs(J);
    for (int j = 0; j < J; ++j) {
        for (int i = 0; i < I; ++i) {
            indices[j].push_back(i);
            coeffs[j].push_back(A[i][j]);
        }
    }
    CPLEX_add_constraints(env, lp, J, R, std::vector<char>(J, 'G'), indices, coeffs, make_names("request", J));

    // Objective sense (minimize)
    CPLEX_call(CPXchgobjsen, env, lp, CPX_MIN);

    // Write .lp to check if the model is correct
    CPLEX_call(CPXwriteprob, env, lp, "minimum_cost_covering.lp", nullptr);
}
}  // namespace

auto main() -> int {
    auto exit_code = EXIT_SUCCESS;
    CPXENVptr env{nullptr};
    CPXLPptr lp{nullptr};

    try {
        std::tie(env, lp) = CPLEX_open("cost_covering");
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
