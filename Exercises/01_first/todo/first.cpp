/**
 * @file first.cpp
 * @brief basic use of newcols and addrow
 * to solve the model
 *  max 2 x1 + 3 x2 + w
 *    x1 + x2 <= 5 z ---> 		x1 + x2 - 5 z <= 0
 *    x2 + 9 y1 + 9 y2 + 8w = 2
 *    8 y1 >= -1
 *    -4 y1 + 7z + 5w <= 9
 *    x1,x2 >=0
 *    y1 <=0
 *    z in {0,1}
 *    w in Z+
 */

#include <cstdio>
#include <iostream>
#include <string>
#include <vector>

#include "../../../cpxmacro.h"

using namespace std;

// error status and messagge buffer
int status;
char errmsg[BUF_SIZE];

int main(int argc, char const* argv[]) {
    try {
        ///////////////////////// init env and lp prob
        //
        //	  CPXENVptr CPXopenCPLEX   ( status_out )
        //	  CPXLPptr  CPXcreateprob  ( env, status_out, probname )
        //
        /// TODO...

        ///////////////////////// create variables with newcols
        //
        //    status = CPXnewcols (env, lp, ccnt, obj, lb, ub, xctype, colname)
        //
        /// TODO...

        ///////////////////////// create constraints
        //
        //    status = CPXaddrows (env, lp, colcnt, rowcnt, nzcnt, rhs, sense, rmatbeg, rmatind, rmatval , newcolname, newrowname)
        //
        /// TODO...

        ///////////////////////// set optimization sense
        //
        //    status = CPXchgobjsen (env, lp, CPX_MAX/CPX_MIN);
        //
        /// TODO...

        ///////////////////////// print problem (debug)
        //
        //    status = CPXwriteprob (env, lp, filename, fileformat)
        //
        /// TODO...

        ///////////////////////// optimize!
        //
        //    status = CPXmipopt (env, lp);
        //
        /// TODO...

        ///////////////////////// print solution (objective function and variables
        //
        //    status = CPXgetobjval (env, lp, obj_func_value_out )
        //	  int CPXgetnumcols(env, lp)
        //    status = CPXgetx (env, lp, varVals_out, fromIdx, toIdx)
        //
        /// TODO...

        ///////////////////////// export solution
        //
        //    status = CPXsolwrite (env, lp, filename);
        //
        /// TODO...

        ///////////////////////// close and free problem and env
        //
        //    CPXfreeprob (env, problem)    -     CPXcloseCPLEX (env)
        //
        /// TODO...

    } catch (std::exception& e) {
        std::cout << ">>>EXCEPTION: " << e.what() << std::endl;
    }
    return 0;
}
