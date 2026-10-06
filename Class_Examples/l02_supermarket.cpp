#include <algorithm>
#include <cstdlib>
#include <exception>
#include <iostream>
#include <string>
#include <tuple>
#include <vector>

#include "cplex_utils.hpp"

// A supermarket chain has a budget W available for opening new stores.
// Preliminary analyses identified a set I of possible locations. Opening a
// store in i € I has a fixed cost Fi (land acquisition, other administrative
// costs etc.) and a variable cost Ci per 100 m2 of store. Once opened, the
// store in i guarantees a revenue of Ri per 100 m2. Determine the subset of
// location where a store has to be opened and the related size in order to
// maximize the total revenue.

// I   : candidate locations
// W   : available budget
// F_i : fixed cost for opening a store in i in I
// C_i : variable cost per 100 m2 of store in i in I
// R_i : revenue per 100 m2 of store in i in I
// M   : big M, upper bound on the size of any store

// == Variables
// x_i : size of the store in i in I (in 100 m2)
// y_i : 1 if a store is opened in i in I, 0 otherwise

// == Model
// max sum_{i in I} R_i x_i
// s.t.
// sum_{i in I} (C_i x_i + F_i y_i) <= W
// x_i <= M y_i                     forall i in I
// x_i in R_+                       forall i in I
// y_i in {0,1}                     forall i in I

namespace {
// Data
constexpr int I = 4;                              // candidate locations
constexpr double W = 1000;                        // available budget
const std::vector<double> F{200, 150, 300, 100};  // F[i] = fixed cost for opening a store in i in I
const std::vector<double> C{40, 50, 30, 60};      // C[i] = variable cost per 100 m2 of store in i in I
const std::vector<double> R{90, 100, 75, 110};    // R[i] = revenue per 100 m2 of store in i in I
const double M = W / std::ranges::min(C);         // big M: no store can be larger than the budget allows

// Variable indices
constexpr auto x(int i) -> int { return i; }
constexpr auto y(int i) -> int { return I + i; }

auto CPLEX_write_lp(CPXENVptr env, CPXLPptr lp) -> void {
    // Adding the variables
    CPLEX_add_variables(env, lp, I, R, {}, {}, {}, make_names("x", I));
    CPLEX_add_variables(env, lp, I, {}, {}, {}, std::vector<char>(I, 'B'), make_names("y", I));

    // Adding the constraints
    std::vector<int> indices;
    std::vector<double> coeffs;
    for (int i = 0; i < I; ++i) {
        indices.push_back(x(i));
        coeffs.push_back(C[i]);
        indices.push_back(y(i));
        coeffs.push_back(F[i]);
    }
    CPLEX_add_constraint(env, lp, W, 'L', indices, coeffs, "budget");

    std::vector<std::vector<int>> big_M_indices;
    std::vector<std::vector<double>> big_M_coeffs;
    for (int i = 0; i < I; ++i) {
        big_M_indices.push_back({x(i), y(i)});
        big_M_coeffs.push_back({1, -M});
    }
    CPLEX_add_constraints(env, lp, I, std::vector<double>(I, 0), std::vector<char>(I, 'L'), big_M_indices, big_M_coeffs, make_names("big_M", I));

    // Objective sense (maximize)
    CPLEX_call(CPXchgobjsen, env, lp, CPX_MAX);

    // Write .lp to check if the model is correct
    CPLEX_call(CPXwriteprob, env, lp, "supermarket.lp", nullptr);
}
}  // namespace

auto main() -> int {
    auto exit_code = EXIT_SUCCESS;
    CPXENVptr env{nullptr};
    CPXLPptr lp{nullptr};

    try {
        std::tie(env, lp) = CPLEX_open("supermarket");
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
