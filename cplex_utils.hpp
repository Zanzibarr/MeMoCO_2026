#pragma once

#include <cplex.h>

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

inline auto CPLEX_close(CPXENVptr& env, CPXLPptr& lp) -> void {
    CPLEX_call(CPXfreeprob, env, &lp);
    if (const int cpx_status = CPXcloseCPLEX(&env); cpx_status != 0) {
        CPLEX_throw(nullptr, cpx_status, __FILE__, __LINE__);
    }
}

inline auto CPLEX_add_variable(CPXENVptr env, CPXLPptr lp, double obj, double lb, double ub, char type, char* name) -> void {
    CPLEX_call(CPXnewcols, env, lp, 1, &obj, &lb, &ub, &type, &name);
}

inline auto CPLEX_add_constraint(CPXENVptr env, CPXLPptr lp, double rhs, char sense, const std::vector<int>& indices,
                                 const std::vector<double>& coeffs, char* name) -> void {
    int begin{0};
    CPLEX_call(CPXaddrows, env, lp, 0, 1, std::ssize(indices), &rhs, &sense, &begin, indices.data(), coeffs.data(), nullptr, &name);
};

inline auto print_solution(CPXENVptr env, CPXLPptr lp) -> void {
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