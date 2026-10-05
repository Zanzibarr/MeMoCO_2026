#include <array>
#include <cstdlib>
#include <exception>
#include <format>
#include <iostream>
#include <string>
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
constexpr int I = 3;                            // vegetables, meat and fruit
constexpr int J = 3;                            // proteins, iron and calcium
constexpr std::array<double, I> C{4, 10, 7};    // C[i] = unit cost of resource i in I
constexpr std::array<double, J> R{20, 30, 10};  // R[j] = requested amount of j in J
constexpr std::array<std::array<double, J>, I> A{{
    {5, 6, 5},
    {15, 10, 3},
    {4, 5, 12},
}};  // A[i][j] = amount of request j in J satisfied by one unit of resource i in I

auto CPLEX_write_lp(CPXENVptr env, CPXLPptr lp) -> void {
    // Adding the variables (x_i has index i)
    for (int i = 0; i < I; ++i) {
        CPLEX_add_variable(env, lp, C[i], 0, CPX_INFBOUND, 'C', std::format("x_{}", i).data());
    }

    // Adding the constraints
    for (int j = 0; j < J; ++j) {
        std::vector<int> indices;
        std::vector<double> coeffs;
        for (int i = 0; i < I; ++i) {
            indices.push_back(i);
            coeffs.push_back(A[i][j]);
        }
        CPLEX_add_constraint(env, lp, R[j], 'G', indices, coeffs, std::format("request_{}", j).data());
    }

    // Objective sense (minimize)
    CPLEX_call(CPXchgobjsen, env, lp, CPX_MIN);

    // Write .lp to check if the model is correct
    CPLEX_call(CPXwriteprob, env, lp, "test.lp", nullptr);
}
}  // namespace

auto main() -> int {
    auto exit_code = EXIT_SUCCESS;
    auto [env, lp] = CPLEX_open("cost_covering");

    try {
        CPLEX_write_lp(env, lp);

        CPLEX_call(CPXmipopt, env, lp);

        CPLEX_print_solution(env, lp);
    } catch (const std::exception& e) {
        std::cerr << e.what();
        exit_code = EXIT_FAILURE;
    }

    CPLEX_close(env, lp);
    return exit_code;
}
