#include <array>
#include <cstdlib>
#include <exception>
#include <format>
#include <iostream>
#include <string>
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
// max sum_{i in J} P_j x_j
// s.t.
// sum_{j in J} Q_ij x_j <= D_i     forall i in I
// x_j in R_+/Z_+/{0,1} (depends on the type of product)       forall j in J

namespace {
// Data (with the perfumes example)
constexpr int I = 3;                           // rose, lily and violet
constexpr int J = 2;                           // perfume 1 and perfume 2
constexpr std::array<double, I> D{27, 21, 9};  // D[i] = availability of resource i in I
constexpr std::array<double, J> P{130, 100};   // P[j] = profit of product j in J
constexpr std::array<std::array<double, J>, I> Q{{
    {1.5, 1},
    {1, 1},
    {0.3, 0.5},
}};  // Q[i][j] = amount of resource i in I required for each unit of product j in J

auto write_lp(CPXENVptr env, CPXLPptr lp) -> void {
    // Adding the variables (x_j has index j)
    for (int j = 0; j < J; ++j) {
        CPLEX_add_variable(env, lp, P[j], 0, CPX_INFBOUND, 'C', std::format("x_{}", j).data());
    }

    // Adding the constraints
    for (int i = 0; i < I; ++i) {
        std::vector<int> indices;
        std::vector<double> coeffs;
        for (int j = 0; j < J; ++j) {
            indices.push_back(j);
            coeffs.push_back(Q[i][j]);
        }
        CPLEX_add_constraint(env, lp, D[i], 'L', indices, coeffs, std::format("resource_{}", i).data());
    }

    // Objective sense (maximize)
    CPLEX_call(CPXchgobjsen, env, lp, CPX_MAX);

    // Write .lp to check if the model is correct
    CPLEX_call(CPXwriteprob, env, lp, "test.lp", nullptr);
}
}  // namespace

auto main() -> int {
    auto exit_code = EXIT_SUCCESS;
    auto [env, lp] = CPLEX_open("production_mix");

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
