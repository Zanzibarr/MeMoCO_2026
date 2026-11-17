/**
 * @file antennas.cpp
 * @brief antennas location
 *
 * Antennas can be installed in the sites I to cover the areas J. The signal
 * of an antenna in i received in area j has intensity S_ij: area j is covered
 * if it receives at least one signal with intensity >= T, but more than N such
 * signals interfere. Maximize the number of covered areas without interference.
 *
 * == Parameters
 * I_j = { i in I : S_ij >= T }   antennas that reach area j
 * M_j = |I_j|                    big M: at most M_j antennas reach area j
 *
 * == Variables
 * x_i : 1 if an antenna is installed in i in I, 0 otherwise
 * z_j : 1 if area j in J is covered (without interference), 0 otherwise
 *
 * == Model
 * max sum_{j in J} z_j
 * s.t.
 * sum_{i in I_j} x_i >= z_j                   forall j in J   (covering)
 * sum_{i in I_j} x_i <= N + M_j (1 - z_j)     forall j in J   (no interference)
 * x_i, z_j in {0,1}
 *
 * Differences with the Python version (antennas.py):
 * - here N = 1, in Python N = 2;
 * - here M_j = |I_j|, in Python the tighter M_j = |I_j| - N;
 * - the bridge constraint x_E <= x_D (an antenna in E needs one in D) exists only in Python.
 * Hence the optimal values differ: 5 covered areas here, 6 in Python.
 */

#include <cstdio>
#include <iostream>
#include <vector>

#include "../../../cpxmacro.h"

using namespace std;

// error status and message buffer
int status;
char errmsg[BUF_SIZE];

// data
const int I = 5;
const int J = 6;
const char nameI[I] = {'A', 'B', 'C', 'D', 'E'};
const char nameJ[J] = {'1', '2', '3', '4', '5', '6'};

// signal intensity S(site, area), LINEARIZED as S[i*J + j]
//                         1     2     3     4     5     6
const double S[I * J] = {10.0, 20.0, 16.0, 25.0, 0.0,  10.0,   // A
                         0.0,  12.0, 18.0, 23.0, 11.0, 6.0,    // B
                         21.0, 8.0,  5.0,  6.0,  23.0, 19.0,   // C
                         16.0, 15.0, 15.0, 8.0,  14.0, 18.0,   // D
                         21.0, 13.0, 13.0, 17.0, 18.0, 22.0};  // E

const double T = 18.0;
const double N = 1.0;
double M[J];  // to be initialized

const int NAME_SIZE = 512;
char name[NAME_SIZE];

void setupLP(CEnv env, Prob lp, int& numVars) {
    // intialize parameters
    for (int j = 0; j < J; ++j) {
        M[j] = 0.0;
        for (int i = 0; i < I; ++i) {
            if (S[i * J + j] >= T) M[j] += 1.0;
        }
        std::cout << "M[" << j << "] = " << M[j] << '\t';
    }
    std::cout << endl;
    // add x vars
    for (int i = 0; i < I; ++i) {
        char xtype = 'B';
        double obj = 0.0;
        double lb = 0.0;
        double ub = 1.0;
        snprintf(name, NAME_SIZE, "x_%c", nameI[i]);
        char* xname = (char*)(&name[0]);
        CHECKED_CPX_CALL(CPXnewcols, env, lp, 1, &obj, &lb, &ub, &xtype, &xname);
    }
    // add z vars
    for (int j = 0; j < J; ++j) {
        char ztype = 'B';
        double obj = 1.0;
        double lb = 0.0;
        double ub = 1.0;
        snprintf(name, NAME_SIZE, "z_%c", nameJ[j]);
        char* zname = (char*)(&name[0]);
        CHECKED_CPX_CALL(CPXnewcols, env, lp, 1, &obj, &lb, &ub, &ztype, &zname);
    }
    numVars = CPXgetnumcols(env, lp);
    // add covering constraints
    for (int j = 0; j < J; ++j) {
        std::vector<int> idx;
        std::vector<double> coef;
        for (int i = 0; i < I; i++) {
            if (S[i * J + j] < T) continue;
            idx.push_back(i);
            coef.push_back(1.0);
        }
        idx.push_back(I + j);
        coef.push_back(-1.0);
        char sense = 'G';
        double rhs = 0.0;
        int matbeg = 0;
        CHECKED_CPX_CALL(CPXaddrows, env, lp, 0, 1, idx.size(), &rhs, &sense, &matbeg, &idx[0], &coef[0], NULL, NULL);
    }
    // add tolerance constraints
    for (int j = 0; j < J; ++j) {
        std::vector<int> idx;
        std::vector<double> coef;
        for (int i = 0; i < I; i++) {
            if (S[i * J + j] < T) continue;
            idx.push_back(i);
            coef.push_back(1.0);
        }
        idx.push_back(I + j);
        coef.push_back(M[j]);
        char sense = 'L';
        double rhs = N + M[j];
        int matbeg = 0;
        CHECKED_CPX_CALL(CPXaddrows, env, lp, 0, 1, idx.size(), &rhs, &sense, &matbeg, &idx[0], &coef[0], NULL, NULL);
    }
    CPXchgobjsen(env, lp, CPX_MAX);
    // print (debug)
    CHECKED_CPX_CALL(CPXwriteprob, env, lp, "antennas.lp", 0);
}

int main(int argc, char const* argv[]) {
    try {
        // init
        DECL_ENV(env);
        DECL_PROB(env, lp);
        // setup LP
        int numVars;
        setupLP(env, lp, numVars);
        // optimize
        CHECKED_CPX_CALL(CPXmipopt, env, lp);
        // print
        double objval;
        CHECKED_CPX_CALL(CPXgetobjval, env, lp, &objval);
        std::cout << "Objval: " << objval << std::endl;
        int n = CPXgetnumcols(env, lp);
        cout << n << " " << numVars << endl;
        if (n != numVars) {
            throw std::runtime_error(std::string(__FILE__) + ":" + STRINGIZE(__LINE__) + ": " + "different number of variables");
        }
        std::vector<double> varVals;
        varVals.resize(n);
        CHECKED_CPX_CALL(CPXgetx, env, lp, &varVals[0], 0, n - 1);
        for (int i = 0; i < n; ++i) {
            std::cout << "var in position " << i << " : " << varVals[i] << std::endl;
        }
        CHECKED_CPX_CALL(CPXsolwrite, env, lp, "antennas.sol");
        // free
        CPXfreeprob(env, &lp);
        CPXcloseCPLEX(&env);
    } catch (std::exception& e) {
        std::cout << ">>>EXCEPTION: " << e.what() << std::endl;
    }
    return 0;
}
