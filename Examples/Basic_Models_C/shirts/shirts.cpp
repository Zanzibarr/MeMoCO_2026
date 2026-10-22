/**
 * @file shirts.cpp
 * @brief
 *
 * A group of kids wants to earn as much as possible by selling decorated
 * t-shirts and bags to their friends. There are 10 cotton t-shirts and 15
 * canvas bags available and, for decoration, 32 designed DTF stickers and 40
 * red trims. Each t-shirt gets 6 stickers and 2 trims, and each bag gets
 * 3 stickers and 5 trims. There are also 15 buttons available, and each bag
 * uses two of them for its closure. Moreover, 22 labels have been prepared,
 * to be applied one on each t-shirt and two on each bag. Considering that
 * each decorated t-shirt is sold for 24 Euros and each bag for 16 Euros, and
 * that the friends will buy all t-shirts and bags, determine the production
 * that maximizes the revenue.
 *
 * == Variables
 * x_s, x_b = # t-shirts/bags produced
 *
 * == Model
 * max 24 * x_s + 16 * x_b
 * s.t.
 *          x_s            <= 10 # t-shirts
 *                     x_b <= 15 # bags
 *      6 * x_s +  3 * x_b <= 32 # stickers
 *      2 * x_s +  5 * x_b <= 40 # trims
 *                 2 * x_b <= 15 # buttons
 *      1 * x_s +  2 * x_b <= 22 # labels
 * x_s, x_b in Z_+
 */

#include <iostream>
#include <string>
#include <vector>

#include "../cpxmacro.h"

using namespace std;

// error status and message buffer
int status;
char errmsg[BUF_SIZE];

// adds the row: sum_k coef[k] * x[idx[k]] (sense) rhs
void addRow(CEnv env, Prob lp, vector<int> idx, vector<double> coef, char sense, double rhs, string name) {
    int matbeg = 0;
    char* rowname = (char*)name.c_str();
    CHECKED_CPX_CALL(CPXaddrows, env, lp, 0, 1, idx.size(), &rhs, &sense, &matbeg, &idx[0], &coef[0], NULL, &rowname);
    /// status =      CPXaddrows (        env, lp, colcnt, rowcnt, nzcnt     , rhs  , sense , rmatbeg, rmatind, rmatval , newcolname, newrowname);
}

int main() {
    try {
        // Model creation
        DECL_ENV(env);
        DECL_PROB(env, lp);
        CHECKED_CPX_CALL(CPXchgprobname, env, lp, "shirts");

        // Variables creation (column index of each variable)
        const int x_s = 0;
        const int x_b = 1;
        vector<string> names = {"x_s", "x_b"};
        for (const string& n : names) {
            double lb = 0.0;
            double ub = CPX_INFBOUND;
            char xtype = 'I';  // integer variable -> the problem becomes a MIP
            char* colname = (char*)n.c_str();
            CHECKED_CPX_CALL(CPXnewcols, env, lp, 1, NULL, &lb, &ub, &xtype, &colname);
            /// status =      CPXnewcols (env, lp, ccnt, obj , lb  , ub , xctype, colname);
        }

        // Constraints
        addRow(env, lp, {x_s}, {1}, 'L', 10, "tshirts");
        addRow(env, lp, {x_b}, {1}, 'L', 15, "bags");
        addRow(env, lp, {x_s, x_b}, {6, 3}, 'L', 32, "stickers");
        addRow(env, lp, {x_s, x_b}, {2, 5}, 'L', 40, "trims");
        addRow(env, lp, {x_b}, {2}, 'L', 15, "buttons");
        addRow(env, lp, {x_s, x_b}, {1, 2}, 'L', 22, "labels");

        // Objective function
        vector<int> objidx = {x_s, x_b};
        vector<double> objcoef = {24, 16};
        CHECKED_CPX_CALL(CPXchgobj, env, lp, objidx.size(), &objidx[0], &objcoef[0]);
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
