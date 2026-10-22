/**
 * @file supermarket.cpp
 * @brief
 *
 * A supermarket chain has a budget W available for opening new stores.
 * Preliminary analyses identified a set I of possible locations. Opening a
 * store in i € I has a fixed cost Fi (land acquisition, other administrative
 * costs etc.) and a variable cost Ci per 100 m2 of store. Once opened, the
 * store in i guarantees a revenue of Ri per 100 m2. Determine the subset of
 * location where a store has to be opened and the related size in order to
 * maximize the total revenue.
 *
 * I   : candidate locations
 * W   : available budget
 * F_i : fixed cost for opening a store in i in I
 * C_i : variable cost per 100 m2 of store in i in I
 * R_i : revenue per 100 m2 of store in i in I
 * M   : big M, upper bound on the size of any store
 *
 * == Variables
 * x_i : size of the store in i in I (in 100 m2)
 * y_i : 1 if a store is opened in i in I, 0 otherwise
 *
 * == Model
 * max sum_{i in I} R_i x_i
 * s.t.
 * sum_{i in I} (C_i x_i + F_i y_i) <= W
 * x_i <= M y_i                     forall i in I
 * x_i in R_+                       forall i in I
 * y_i in {0,1}                     forall i in I
 */

#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

#include "../cpxmacro.h"

using namespace std;

// error status and message buffer
int status;
char errmsg[BUF_SIZE];

// data
const int I = 4;                              // candidate locations
const double W = 1000;                        // available budget
const double F[I] = {200, 150, 300, 100};     // F[i] = fixed cost for opening a store in i in I
const double C[I] = {40, 50, 30, 60};         // C[i] = variable cost per 100 m2 of store in i in I
const double R[I] = {90, 100, 75, 110};       // R[i] = revenue per 100 m2 of store in i in I
const double M = W / *min_element(C, C + I);  // big M: no store can be larger than the budget allows

int main() {
    try {
        // Model creation
        DECL_ENV(env);
        DECL_PROB(env, lp);
        CHECKED_CPX_CALL(CPXchgprobname, env, lp, "supermarket");

        // Variables creation: x_i is column i, y_i is column I + i
        vector<string> names;
        for (int i = 0; i < I; ++i) {
            names.push_back("x_" + to_string(i));
            double lb = 0.0;
            double ub = CPX_INFBOUND;
            char xtype = 'C';
            char* colname = (char*)names.back().c_str();
            CHECKED_CPX_CALL(CPXnewcols, env, lp, 1, NULL, &lb, &ub, &xtype, &colname);
            /// status =      CPXnewcols (env, lp, ccnt, obj , lb  , ub , xctype, colname);
        }
        for (int i = 0; i < I; ++i) {
            names.push_back("y_" + to_string(i));
            double lb = 0.0;
            double ub = 1.0;
            char xtype = 'B';  // binary variable -> the problem becomes a MIP
            char* colname = (char*)names.back().c_str();
            CHECKED_CPX_CALL(CPXnewcols, env, lp, 1, NULL, &lb, &ub, &xtype, &colname);
        }

        // Budget constraint [ sum{i} (C_i x_i + F_i y_i) <= W ]
        {
            vector<int> idx;
            vector<double> coef;
            for (int i = 0; i < I; ++i) {
                idx.push_back(i);  // corresponds to variable x_i
                coef.push_back(C[i]);
                idx.push_back(I + i);  // corresponds to variable y_i
                coef.push_back(F[i]);
            }
            char sense = 'L';
            int matbeg = 0;
            const char* rowname = "budget";
            CHECKED_CPX_CALL(CPXaddrows, env, lp, 0, 1, idx.size(), &W, &sense, &matbeg, &idx[0], &coef[0], NULL, (char**)&rowname);
            /// status =      CPXaddrows (        env, lp, colcnt, rowcnt, nzcnt     , rhs  , sense , rmatbeg, rmatind, rmatval , newcolname,
            /// newrowname);
        }

        // Big M constraints [ forall i, x_i - M y_i <= 0 ]
        for (int i = 0; i < I; ++i) {
            vector<int> idx = {i, I + i};  // x_i, y_i
            vector<double> coef = {1.0, -M};
            char sense = 'L';
            double rhs = 0.0;
            int matbeg = 0;
            string name = "big_M_" + to_string(i);
            char* rowname = (char*)name.c_str();
            CHECKED_CPX_CALL(CPXaddrows, env, lp, 0, 1, idx.size(), &rhs, &sense, &matbeg, &idx[0], &coef[0], NULL, &rowname);
        }

        // Objective function [ max sum{i} R_i x_i ]
        for (int i = 0; i < I; ++i) {
            CHECKED_CPX_CALL(CPXchgobj, env, lp, 1, &i, &R[i]);
        }
        CHECKED_CPX_CALL(CPXchgobjsen, env, lp, CPX_MAX);

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
