/**
 * @file farmer.cpp
 * @brief
 *
 * A farmer owns 11 hectares of land where he can grow potatoes or
 * tomatoes. Beyond the land, the available resources are: 70 kg of tomato
 * seeds, 18 tons of potato tubers, 145 tons of fertilizer. The farmer knows
 * that all his production can be sold with a profit of 6000 Euros per hectare
 * of tomatoes and 7000 Euros per hectare of potatoes. Each hectare of tomatoes
 * needs 7 kg seeds and 10 tons fertilizer. Each hectare of potatoes needs
 * 3 tons tubers and 20 tons fertilizer. How does the farmer divide his land
 * in order to gain as much as possible from the available resources?
 *
 * == Variables
 * x_t, x_p = # hectares of tomatoes/potatoes
 *
 * == Model
 * max 6000 * x_t + 7000 * x_p
 * s.t.
 *        1 * x_t +    1 * x_p <=  11 # land
 *        7 * x_t              <=  70 # tomato seeds
 *                     3 * x_p <=  18 # potato tubers
 *       10 * x_t +   20 * x_p <= 145 # fertilizer
 * x_t, x_p in R_+
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
        CHECKED_CPX_CALL(CPXchgprobname, env, lp, "farmer");

        // Variables creation (column index of each variable)
        const int x_t = 0;
        const int x_p = 1;
        vector<string> names = {"x_t", "x_p"};
        for (const string& n : names) {
            double lb = 0.0;
            double ub = CPX_INFBOUND;
            char* colname = (char*)n.c_str();
            CHECKED_CPX_CALL(CPXnewcols, env, lp, 1, NULL, &lb, &ub, NULL, &colname);
            /// status =      CPXnewcols (env, lp, ccnt, obj , lb  , ub , xctype, colname);
            /// xctype = NULL -> continuous variables, the problem stays an LP
        }

        // Constraints
        addRow(env, lp, {x_t, x_p}, {1, 1}, 'L', 11, "land");
        addRow(env, lp, {x_t}, {7}, 'L', 70, "tomato_seeds");
        addRow(env, lp, {x_p}, {3}, 'L', 18, "potato_tubers");
        addRow(env, lp, {x_t, x_p}, {10, 20}, 'L', 145, "fertilizer");

        // Objective function
        vector<int> objidx = {x_t, x_p};
        vector<double> objcoef = {6000, 7000};
        CHECKED_CPX_CALL(CPXchgobj, env, lp, objidx.size(), &objidx[0], &objcoef[0]);
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
