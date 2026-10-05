#include <array>
#include <cstdlib>
#include <exception>
#include <format>
#include <iostream>
#include <string>
#include <vector>

#include "cplex_utils.hpp"

// I    : origins
// J    : destinations
// O_i  : capacity of origin i in I
// D_j  : request of destination j in J
// C_ij : unit transport cost from origin i in I to destination j in J

// == Variables
// x_ij : amount to be transported from i in I to j in J

// == Model
// min sum_{i in I, j in J} C_ij x_ij
// s.t.
// sum_{i in I} x_ij >= D_j        forall j in J
// sum_{j in J} x_ij <= O_i        forall i in I
// x_ij in R_+/Z_+/{0,1}   (depends on the type of unit of transport)  forall i in I, j in J

namespace {
// Data (with the refrigerators example)
constexpr int I = 3;  // factories A, B and C
constexpr int J = 4;  // stores 1, 2, 3 and 4
constexpr std::array<double, I> O{50, 70, 30};
constexpr std::array<double, J> D{20, 60, 30, 40};
constexpr std::array<std::array<double, J>, I> C{{
    {6, 8, 3, 4},
    {4, 2, 1, 3},
    {4, 2, 6, 5},
}};

// Index of variable x_ij
constexpr auto x(int i, int j) -> int { return i * J + j; }

auto write_lp(CPXENVptr env, CPXLPptr lp) -> void {
    // Adding the variables
    for (int i = 0; i < I; ++i) {
        for (int j = 0; j < J; ++j) {
            CPLEX_add_variable(env, lp, C[i][j], 0, CPX_INFBOUND, 'I', std::format("x_{}_{}", i, j).data());
        }
    }

    // Adding the constraints
    for (int j = 0; j < J; ++j) {
        std::vector<int> indices;
        std::vector<double> coeffs;
        for (int i = 0; i < I; ++i) {
            indices.push_back(x(i, j));
            coeffs.push_back(1);
        }
        CPLEX_add_constraint(env, lp, D[j], 'G', indices, coeffs, std::format("request_{}", j).data());
    }
    for (int i = 0; i < I; ++i) {
        std::vector<int> indices;
        std::vector<double> coeffs;
        for (int j = 0; j < J; ++j) {
            indices.push_back(x(i, j));
            coeffs.push_back(1);
        }
        CPLEX_add_constraint(env, lp, O[i], 'L', indices, coeffs, std::format("capacity_{}", i).data());
    }

    // Objective sense (minimize)
    CPLEX_call(CPXchgobjsen, env, lp, CPX_MIN);

    // Write .lp to check if the model is correct
    CPLEX_call(CPXwriteprob, env, lp, "test.lp", nullptr);
}
}  // namespace

auto main() -> int {
    auto exit_code = EXIT_SUCCESS;
    auto [env, lp] = CPLEX_open("transportation");

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
