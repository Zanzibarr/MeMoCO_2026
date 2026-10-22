/**
 * @file optimal_production_mix.cpp
 * @brief
 *
 * I   : resources
 * J   : products
 * D_i : availability of resource i in I
 * P_j : unit profit for product j in J
 * Q_ij: amount of resource i in I required for each unit of product j in J
 *
 * == Variables
 * x_j : amount of product j in J
 *
 * == Model
 * max sum_{j in J} P_j x_j
 * s.t.
 * sum_{j in J} Q_ij x_j <= D_i     forall i in I
 * x_j in R_+/Z_+/{0,1} (depends on the type of product)       forall j in J
 */

#include <iostream>
#include <string>
#include <vector>

#include "../cpxmacro.h"

using namespace std;

// error status and message buffer
int status;
char errmsg[BUF_SIZE];

// data (with the perfumes example)
const int I = 3;                  // rose, lily and violet
const int J = 2;                  // perfume 1 and perfume 2
const double D[I] = {27, 21, 9};  // D[i] = availability of resource i in I
const double P[J] = {130, 100};   // P[j] = profit of product j in J
const double Q[I][J] = {
    {1.5, 1},
    {1, 1},
    {0.3, 0.5},
};  // Q[i][j] = amount of resource i in I required for each unit of product j in J

int main() {
    try {
        // Model creation
        DECL_ENV(env);
        DECL_PROB(env, lp);
        CHECKED_CPX_CALL(CPXchgprobname, env, lp, "production_mix");

        // Variables creation: x_j is column j
        vector<string> names;
        for (int j = 0; j < J; ++j) {
            names.push_back("x_" + to_string(j));
            double lb = 0.0;
            double ub = CPX_INFBOUND;
            char* colname = (char*)names.back().c_str();
            CHECKED_CPX_CALL(CPXnewcols, env, lp, 1, NULL, &lb, &ub, NULL, &colname);
            /// status =      CPXnewcols (env, lp, ccnt, obj , lb  , ub , xctype, colname);
            /// xctype = NULL -> continuous variables, the problem stays an LP
        }

        // Constraints [ forall i, sum{j} Q_ij x_j <= D_i ]
        for (int i = 0; i < I; ++i) {
            vector<int> idx;
            vector<double> coef;
            for (int j = 0; j < J; ++j) {
                idx.push_back(j);  // corresponds to variable x_j
                coef.push_back(Q[i][j]);
            }
            char sense = 'L';
            int matbeg = 0;
            string name = "resource_" + to_string(i);
            char* rowname = (char*)name.c_str();
            CHECKED_CPX_CALL(CPXaddrows, env, lp, 0, 1, idx.size(), &D[i], &sense, &matbeg, &idx[0], &coef[0], NULL, &rowname);
            /// status =      CPXaddrows (        env, lp, colcnt, rowcnt, nzcnt     , rhs  , sense , rmatbeg, rmatind, rmatval , newcolname,
            /// newrowname);
        }

        // Objective function [ max sum{j} P_j x_j ]
        for (int j = 0; j < J; ++j) {
            CHECKED_CPX_CALL(CPXchgobj, env, lp, 1, &j, &P[j]);
        }
        CHECKED_CPX_CALL(CPXchgobjsen, env, lp, CPX_MAX);

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
