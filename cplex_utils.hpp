#pragma once

#include <cplex.h>

#include <array>
#include <cstdio>
#include <cstring>
#include <format>
#include <iostream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

constexpr unsigned int BUF_SIZE = 4096;
inline char err_msg[BUF_SIZE];

/* Throw a runtime_error carrying the Cplex message for cpx_status */
[[noreturn]] inline void CPLEX_throw(CPXCENVptr env, int cpx_status, const char* file, int line) {
    if (CPXgeterrorstring(env, cpx_status, err_msg) == nullptr) {
        std::snprintf(err_msg, BUF_SIZE, "Unknown CPLEX error %d", cpx_status);
    }
    const std::size_t len = std::strlen(err_msg);
    if (len > 0 && err_msg[len - 1] == '\n') {
        err_msg[len - 1] = '\0';
    }
    throw std::runtime_error(std::format("{}:{}: {}", file, line, err_msg));
}

/* Throw if cpx_status (the return value of a Cplex API function) signals an error */
inline auto CPLEX_check(CPXCENVptr env, int cpx_status, const char* file, int line) -> void {
    if (cpx_status != 0) {
        CPLEX_throw(env, cpx_status, file, line);
    }
}

/* Make a checked call to a Cplex API function */
#define CPLEX_call(func, env, ...) CPLEX_check(env, func(env, __VA_ARGS__), __FILE__, __LINE__)

/* Wrapper to open the CPLEX environment and initialize an (empty) problem */
inline auto CPLEX_open(const std::string& lp_name) -> std::pair<CPXENVptr, CPXLPptr> {
    int cpx_status{0};

    CPXENVptr env = CPXopenCPLEX(&cpx_status);
    if (cpx_status != 0) {
        CPLEX_throw(nullptr, cpx_status, __FILE__, __LINE__);
    }

    CPXLPptr lp = CPXcreateprob(env, &cpx_status, lp_name.c_str());
    if (cpx_status != 0) {
        CPXcloseCPLEX(&env);
        CPLEX_throw(nullptr, cpx_status, __FILE__, __LINE__);
    }

    return {env, lp};
}

/* Wrapper to free the resources (nullptr ones are skipped). It never throws, so it can always be called at the end of main: errors are only
 * printed */
inline auto CPLEX_close(CPXENVptr& env, CPXLPptr& lp) noexcept -> void {
    auto report = [](CPXCENVptr e, int cpx_status) {
        if (CPXgeterrorstring(e, cpx_status, err_msg) == nullptr) {
            std::snprintf(err_msg, BUF_SIZE, "Unknown CPLEX error %d\n", cpx_status);
        }
        std::cerr << err_msg;
    };
    if (lp != nullptr) {
        if (const int cpx_status = CPXfreeprob(env, &lp); cpx_status != 0) {
            report(env, cpx_status);
        }
    }
    if (env != nullptr) {
        if (const int cpx_status = CPXcloseCPLEX(&env); cpx_status != 0) {
            report(nullptr, cpx_status);
        }
    }
}

/* Wrapper to add a single variable */
inline auto CPLEX_add_variable(CPXENVptr env, CPXLPptr lp, double obj, double lb, double ub, char type, const std::string& name) -> void {
    // Cplex wants char**: point to a local char* (nullptr = no name)
    char* name_ptr = const_cast<char*>(name.c_str());
    CPLEX_call(CPXnewcols, env, lp, 1, &obj, &lb, &ub, &type, name.empty() ? nullptr : &name_ptr);
}

/* Wrapper to add a set of variables, passing {} instead of a vector uses CPLEX's defaults (0 for objs, 0 for lbs, CPX_INFBOUND for ubs, 'C' for
 * types, ["x1", "x2", ...] for names) */
inline auto CPLEX_add_variables(CPXENVptr env, CPXLPptr lp, size_t n, const std::vector<double>& objs, const std::vector<double>& lbs,
                                const std::vector<double>& ubs, const std::vector<char>& types, const std::vector<std::string>& names) -> void {
    auto check_size = [n](std::size_t size, const char* what) {
        if (size != 0 && size != n) {
            throw std::invalid_argument(std::format("CPLEX_add_variables: {} has size {}, expected {} or 0", what, size, n));
        }
    };
    check_size(objs.size(), "objs");
    check_size(lbs.size(), "lbs");
    check_size(ubs.size(), "ubs");
    check_size(types.size(), "types");
    check_size(names.size(), "names");

    // Cplex wants char**; the pointers stay valid as long as names is alive
    std::vector<char*> name_ptrs;
    name_ptrs.reserve(names.size());
    for (const auto& name : names) {
        name_ptrs.push_back(const_cast<char*>(name.c_str()));
    }

    // Return the vector data, or nullptr if empty so that Cplex applies its defaults
    auto data_or_null = [](auto& v) { return v.empty() ? nullptr : v.data(); };

    CPLEX_call(CPXnewcols, env, lp, static_cast<int>(n), data_or_null(objs), data_or_null(lbs), data_or_null(ubs), data_or_null(types),
               data_or_null(name_ptrs));
}

/* Helper function to generate names for the type ["<base>_0", "<base>_1", ...] */
inline auto make_names(const std::string& base, std::size_t size) -> std::vector<std::string> {
    std::vector<std::string> names(size);
    unsigned int counter{0};
    for (auto& name : names) {
        name = std::format("{}_{}", base, counter++);
    }
    return names;
}

/* Wrapper to add a single constraint */
inline auto CPLEX_add_constraint(CPXENVptr env, CPXLPptr lp, double rhs, char sense, const std::vector<int>& indices,
                                 const std::vector<double>& coeffs, const std::string& name) -> void {
    if (indices.size() != coeffs.size()) {
        throw std::invalid_argument(
            std::format("CPLEX_add_constraint: {} has {} indices and {} coefficients", name, indices.size(), coeffs.size()));
    }
    int begin{0};
    // Cplex wants char**: point to a local char* (nullptr = no name)
    char* name_ptr = const_cast<char*>(name.c_str());
    CPLEX_call(CPXaddrows, env, lp, 0, 1, static_cast<int>(indices.size()), &rhs, &sense, &begin, indices.data(), coeffs.data(), nullptr,
               name.empty() ? nullptr : &name_ptr);
}

/* Wrapper to add a set of constraints, constraint k is sum_t coeffss[k][t] x_{indices[k][t]} <senses[k]> rhss[k]. Passing {} instead of rhss,
 * senses or names uses CPLEX's defaults (0 for rhss, 'E' for senses, ["c1", "c2", ...] for names) */
inline auto CPLEX_add_constraints(CPXENVptr env, CPXLPptr lp, size_t n, const std::vector<double>& rhss, const std::vector<char>& senses,
                                  const std::vector<std::vector<int>>& indices, const std::vector<std::vector<double>>& coeffss,
                                  const std::vector<std::string>& names) -> void {
    auto check_size = [n](std::size_t size, const char* what, bool allow_empty) {
        if ((size != 0 || !allow_empty) && size != n) {
            throw std::invalid_argument(
                std::format("CPLEX_add_constraints: {} has size {}, expected {}{}", what, size, n, allow_empty ? " or 0" : ""));
        }
    };
    check_size(rhss.size(), "rhss", true);
    check_size(senses.size(), "senses", true);
    check_size(indices.size(), "indices", false);
    check_size(coeffss.size(), "coeffss", false);
    check_size(names.size(), "names", true);

    // Cplex wants the rows in a single sparse matrix: rmatbeg[k] is the position in rmatind/rmatval where row k starts
    std::vector<int> rmatbeg;
    std::vector<int> rmatind;
    std::vector<double> rmatval;
    rmatbeg.reserve(n);
    for (std::size_t k = 0; k < n; ++k) {
        if (indices[k].size() != coeffss[k].size()) {
            throw std::invalid_argument(
                std::format("CPLEX_add_constraints: row {} has {} indices and {} coefficients", k, indices[k].size(), coeffss[k].size()));
        }
        rmatbeg.push_back(static_cast<int>(rmatind.size()));
        rmatind.insert(rmatind.end(), indices[k].begin(), indices[k].end());
        rmatval.insert(rmatval.end(), coeffss[k].begin(), coeffss[k].end());
    }

    // Cplex wants char**; the pointers stay valid as long as names is alive
    std::vector<char*> name_ptrs;
    name_ptrs.reserve(names.size());
    for (const auto& name : names) {
        name_ptrs.push_back(const_cast<char*>(name.c_str()));
    }

    // Return the vector data, or nullptr if empty so that Cplex applies its defaults
    auto data_or_null = [](auto& v) { return v.empty() ? nullptr : v.data(); };

    CPLEX_call(CPXaddrows, env, lp, 0, static_cast<int>(n), static_cast<int>(rmatind.size()), data_or_null(rhss), data_or_null(senses), rmatbeg.data(),
               rmatind.data(), rmatval.data(), nullptr, data_or_null(name_ptrs));
}

/* Wrapper to obtain and print the solution found */
inline auto CPLEX_print_solution(CPXENVptr env, CPXLPptr lp) -> void {
    // Check whether a solution exists
    int sol_type{0};
    CPLEX_call(CPXsolninfo, env, lp, nullptr, &sol_type, nullptr, nullptr);
    if (sol_type == CPX_NO_SOLN) {
        std::array<char, CPXMESSAGEBUFSIZE> status{};
        CPXgetstatstring(env, CPXgetstat(env, lp), status.data());
        std::cout << status.data() << '\n';
        return;
    }

    // Objective value
    double obj_val{0};
    CPLEX_call(CPXgetobjval, env, lp, &obj_val);

    // Variable values
    const int var_cnt = CPXgetnumcols(env, lp);
    std::vector<double> x(var_cnt);
    CPLEX_call(CPXgetx, env, lp, x.data(), 0, var_cnt - 1);

    std::cout << "===================================================" << '\n';
    std::cout << "Objective function value: " << obj_val << '\n';
    for (int i = 0; i < var_cnt; ++i) {
        // Retrieve the variable name (surplus = 0 means the buffer was large enough)
        char* name{nullptr};
        std::array<char, BUF_SIZE> name_buf{};
        int surplus{0};
        CPLEX_call(CPXgetcolname, env, lp, &name, name_buf.data(), BUF_SIZE, &surplus, i, i);
        std::cout << name << " " << x.at(i) << '\n';
    }
    std::cout << "===================================================" << '\n';
}