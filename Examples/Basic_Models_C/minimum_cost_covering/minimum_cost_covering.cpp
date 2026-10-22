/**
 * @file minimum_cost_covering.cpp
 * @brief
 *
 * I   : resources
 * J   : requests
 * C_i : unit cost of resource i in I
 * R_j : requested amount of j in J
 * A_ij: amount of request j in J satisfied by one unit of resource i in I
 *
 * == Variables
 * x_i : amount of resource i in I
 *
 * == Model
 * min sum_{i in I} C_i x_i
 * s.t.
 * sum_{i in I} A_ij x_i >= R_j     forall j in J
 * x_i in R_+/Z_+/{0,1} (depends on the type of resource)       forall i in I
 */

#include <iostream>
#include <string>
#include <vector>

#include "../cpxmacro.h"

using namespace std;

// error status and message buffer
int status;
char errmsg[BUF_SIZE];

// data (with the diet example)
const int I = 3;                   // vegetables, meat and fruit
const int J = 3;                   // proteins, iron and calcium
const double C[I] = {4, 10, 7};    // C[i] = unit cost of resource i in I
const double R[J] = {20, 30, 10};  // R[j] = requested amount of j in J
const double A[I][J] = {
    {5, 6, 5},
    {15, 10, 3},
    {4, 5, 12},
};  // A[i][j] = amount of request j in J satisfied by one unit of resource i in I

int main() {
    try {
        // Model creation
        DECL_ENV(env);
        DECL_PROB(env, lp);
        CHECKED_CPX_CALL(CPXchgprobname, env, lp, "cost_covering");

        // Variables creation: x_i is column i
        vector<string> names;
        for (int i = 0; i < I; ++i) {
            names.push_back("x_" + to_string(i));
            double lb = 0.0;
            double ub = CPX_INFBOUND;
            char* colname = (char*)names.back().c_str();
            CHECKED_CPX_CALL(CPXnewcols, env, lp, 1, NULL, &lb, &ub, NULL, &colname);
            /// status =      CPXnewcols (env, lp, ccnt, obj , lb  , ub , xctype, colname);
            /// xctype = NULL -> continuous variables, the problem stays an LP
        }

        // Constraints [ forall j, sum{i} A_ij x_i >= R_j ]
        for (int j = 0; j < J; ++j) {
            vector<int> idx;
            vector<double> coef;
            for (int i = 0; i < I; ++i) {
                idx.push_back(i);  // corresponds to variable x_i
                coef.push_back(A[i][j]);
            }
            char sense = 'G';
            int matbeg = 0;
            string name = "request_" + to_string(j);
            char* rowname = (char*)name.c_str();
            CHECKED_CPX_CALL(CPXaddrows, env, lp, 0, 1, idx.size(), &R[j], &sense, &matbeg, &idx[0], &coef[0], NULL, &rowname);
            /// status =      CPXaddrows (        env, lp, colcnt, rowcnt, nzcnt     , rhs  , sense , rmatbeg, rmatind, rmatval , newcolname,
            /// newrowname);
        }

        // Objective function [ min sum{i} C_i x_i ]
        for (int i = 0; i < I; ++i) {
            CHECKED_CPX_CALL(CPXchgobj, env, lp, 1, &i, &C[i]);
        }
        CHECKED_CPX_CALL(CPXchgobjsen, env, lp, CPX_MIN);

        // Optimization (CPX_OFF disables solver output)
        CHECKED_CPX_CALL(CPXsetintparam, env, CPX_PARAM_SCRIND, CPX_OFF);
        CHECKED_CPX_CALL(CPXlpopt, env, lp);

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
