/**
 * @file transportation.cpp
 * @brief
 *
 * I    : origins
 * J    : destinations
 * O_i  : capacity of origin i in I
 * D_j  : request of destination j in J
 * C_ij : unit transport cost from origin i in I to destination j in J
 *
 * == Variables
 * x_ij : amount to be transported from i in I to j in J
 *
 * == Model
 * min sum_{i in I, j in J} C_ij x_ij
 * s.t.
 * sum_{i in I} x_ij >= D_j        forall j in J
 * sum_{j in J} x_ij <= O_i        forall i in I
 * x_ij in R_+/Z_+/{0,1}   (depends on the type of unit of transport)  forall i in I, j in J
 */

#include <iostream>
#include <string>
#include <vector>

#include "../cpxmacro.h"

using namespace std;

// error status and message buffer
int status;
char errmsg[BUF_SIZE];

// data (with the refrigerators example)
const int I = 3;  // factories A, B and C
const int J = 4;  // stores 1, 2, 3 and 4
const double O[I] = {50, 70, 30};
const double D[J] = {20, 60, 30, 40};
const double C[I][J] = {{6, 8, 3, 4}, {4, 2, 1, 3}, {4, 2, 6, 5}};

// column index of variable x_ij (LINEARIZED: just an implementation choice!)
int xIdx(int i, int j) { return i * J + j; }

int main() {
    try {
        // Model creation
        DECL_ENV(env);
        DECL_PROB(env, lp);
        CHECKED_CPX_CALL(CPXchgprobname, env, lp, "transportation");

        // Variables creation: x_ij is column xIdx(i, j)
        vector<string> names;
        for (int i = 0; i < I; ++i) {
            for (int j = 0; j < J; ++j) {
                names.push_back("x_" + to_string(i) + "_" + to_string(j));
                double lb = 0.0;
                double ub = CPX_INFBOUND;
                char xtype = 'I';  // integer variable -> the problem becomes a MIP
                char* colname = (char*)names.back().c_str();
                CHECKED_CPX_CALL(CPXnewcols, env, lp, 1, NULL, &lb, &ub, &xtype, &colname);
                /// status =      CPXnewcols (env, lp, ccnt, obj , lb  , ub , xctype, colname);
            }
        }

        // Request constraints (destinations) [ forall j, sum{i} x_ij >= D_j ]
        for (int j = 0; j < J; ++j) {
            vector<int> idx;
            vector<double> coef;
            for (int i = 0; i < I; ++i) {
                idx.push_back(xIdx(i, j));  // corresponds to variable x_ij
                coef.push_back(1.0);
            }
            char sense = 'G';
            int matbeg = 0;
            string name = "request_" + to_string(j);
            char* rowname = (char*)name.c_str();
            CHECKED_CPX_CALL(CPXaddrows, env, lp, 0, 1, idx.size(), &D[j], &sense, &matbeg, &idx[0], &coef[0], NULL, &rowname);
            /// status =      CPXaddrows (        env, lp, colcnt, rowcnt, nzcnt     , rhs  , sense , rmatbeg, rmatind, rmatval , newcolname,
            /// newrowname);
        }

        // Capacity constraints (origins) [ forall i, sum{j} x_ij <= O_i ]
        for (int i = 0; i < I; ++i) {
            vector<int> idx;
            vector<double> coef;
            for (int j = 0; j < J; ++j) {
                idx.push_back(xIdx(i, j));  // corresponds to variable x_ij
                coef.push_back(1.0);
            }
            char sense = 'L';
            int matbeg = 0;
            string name = "capacity_" + to_string(i);
            char* rowname = (char*)name.c_str();
            CHECKED_CPX_CALL(CPXaddrows, env, lp, 0, 1, idx.size(), &O[i], &sense, &matbeg, &idx[0], &coef[0], NULL, &rowname);
        }

        // Objective function [ min sum{i,j} C_ij x_ij ]
        for (int i = 0; i < I; ++i) {
            for (int j = 0; j < J; ++j) {
                int idx = xIdx(i, j);
                CHECKED_CPX_CALL(CPXchgobj, env, lp, 1, &idx, &C[i][j]);
            }
        }
        CHECKED_CPX_CALL(CPXchgobjsen, env, lp, CPX_MIN);

        // Optimization (CPX_OFF disables solver output)
        CHECKED_CPX_CALL(CPXsetintparam, env, CPX_PARAM_SCRIND, CPX_OFF);
        CHECKED_CPX_CALL(CPXmipopt, env, lp);

        // Print solution
        int solntype;
        CHECKED_CPX_CALL(CPXsolninfo, env, lp, NULL, &solntype, NULL, NULL);
        if (solntype != CPX_NO_SOLN) {
            double objval;
            CHECKED_CPX_CALL(CPXgetobjval, env, lp, &objval);
            int n = CPXgetnumcols(env, lp);
            vector<double> x(n);
            CHECKED_CPX_CALL(CPXgetx, env, lp, &x[0], 0, n - 1);
            cout << "===================================================" << endl;
            cout << "Objective function value: " << objval << endl;
            for (int k = 0; k < n; ++k) cout << names[k] << " " << x[k] << endl;
            cout << "===================================================" << endl;
        } else {
            char statstring[CPXMESSAGEBUFSIZE];
            CPXgetstatstring(env, CPXgetstat(env, lp), statstring);
            cout << statstring << endl;
        }

        // free
        CPXfreeprob(env, &lp);
        CPXcloseCPLEX(&env);
    } catch (std::exception& e) {
        cout << ">>>EXCEPTION: " << e.what() << endl;
    }
    return 0;
}
