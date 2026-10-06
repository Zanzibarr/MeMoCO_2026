#include <cstdlib>
#include <exception>
#include <format>
#include <iostream>
#include <string>
#include <tuple>
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
const std::vector<double> O{50, 70, 30};
const std::vector<double> D{20, 60, 30, 40};
const std::vector<std::vector<double>> C{
    {6, 8, 3, 4},
    {4, 2, 1, 3},
    {4, 2, 6, 5},
};

// Index of variable x_ij
constexpr auto x(int i, int j) -> int { return i * J + j; }

auto CPLEX_write_lp(CPXENVptr env, CPXLPptr lp) -> void {
    // Adding the variables
    for (int i = 0; i < I; ++i) {
        CPLEX_add_variables(env, lp, J, C[i], {}, {}, std::vector<char>(J, 'I'), make_names(std::format("x_{}", i), J));
    }

    // Adding the request constraints (one per destination j)
    std::vector<std::vector<int>> request_indices(J);
    for (int j = 0; j < J; ++j) {
        for (int i = 0; i < I; ++i) {
            request_indices[j].push_back(x(i, j));
        }
    }
    CPLEX_add_constraints(env, lp, J, D, std::vector<char>(J, 'G'), request_indices, std::vector<std::vector<double>>(J, std::vector<double>(I, 1)),
                          make_names("request", J));

    // Adding the capacity constraints (one per origin i)
    std::vector<std::vector<int>> capacity_indices(I);
    for (int i = 0; i < I; ++i) {
        for (int j = 0; j < J; ++j) {
            capacity_indices[i].push_back(x(i, j));
        }
    }
    CPLEX_add_constraints(env, lp, I, O, std::vector<char>(I, 'L'), capacity_indices, std::vector<std::vector<double>>(I, std::vector<double>(J, 1)),
                          make_names("capacity", I));

    // Objective sense (minimize)
    CPLEX_call(CPXchgobjsen, env, lp, CPX_MIN);

    // Write .lp to check if the model is correct
    CPLEX_call(CPXwriteprob, env, lp, "transportation.lp", nullptr);
}
}  // namespace

auto main() -> int {
    auto exit_code = EXIT_SUCCESS;
    CPXENVptr env{nullptr};
    CPXLPptr lp{nullptr};

    try {
        std::tie(env, lp) = CPLEX_open("transportation");
        CPLEX_write_lp(env, lp);

        CPLEX_call(CPXmipopt, env, lp);

        CPLEX_print_solution(env, lp);
    } catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
        exit_code = EXIT_FAILURE;
    }

    CPLEX_close(env, lp);
    return exit_code;
}
