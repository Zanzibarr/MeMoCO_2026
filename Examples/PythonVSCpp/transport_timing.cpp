/**
 * @file transport_timing.cpp
 * @brief
 */

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

#include "cpxmacro.h"

using namespace std;

// error status and messagge buffer
int status;
char errmsg[BUF_SIZE];

// data
const int I = 700;
const int J = 1000;
char nameI[I];    // origins
char nameJ[J];    // destinations
double D[I];      // Availability (origins)
double R[J];      // Demand (destinations)
double C[I * J];  // costs(origin, destination) LINEARIZED (just an implementation choice!)

const double K = 10000.0;
const double F = 50.0;
const double N = 4.0;
const double L = 65.0;

const int NAME_SIZE = 512;
char name[NAME_SIZE];

std::vector<std::vector<int> > map_x;

void setupLP(CEnv env, Prob lp, double costThreshold) {
    double timestamp1, timestamp2;
    std::cout << "\ncreating maps...";
    std::cout.flush();
    CHECKED_CPX_CALL(CPXgettime, env, &timestamp1);
    map_x.resize(I);
    for (int i = 0; i < I; ++i) {
        map_x[i].resize(J);
        for (int j = 0; j < J; ++j) {
            map_x[i][j] = -1;
        }
    }

    CHECKED_CPX_CALL(CPXgettime, env, &timestamp2);
    std::cout << "time: " << timestamp2 - timestamp1;
    std::cout.flush();
    std::cout << "\ncreating variables...";
    std::cout.flush();
    CHECKED_CPX_CALL(CPXgettime, env, &timestamp1);
    int current_var_position = 0;
    // add x vars [in o.f.: sum{i,j} C_ij x_ij + ...]
    for (int i = 0; i < I; i++) {
        for (int j = 0; j < J; j++) {
            if (C[i * J + j] > costThreshold) continue;

            char xtype = 'C';
            double lb = 0.0;
            double ub = CPX_INFBOUND;
            snprintf(name, NAME_SIZE, "x_%c_%c", nameI[i], nameJ[j]);
            char* xname = (char*)(&name[0]);
            CHECKED_CPX_CALL(CPXnewcols, env, lp, 1, &C[i * J + j], &lb, &ub, &xtype, &xname);
            /// status =      CPXnewcols (env, lp, ccnt, obj      , lb  , ub, xctype, colname);
            map_x[i][j] = current_var_position++;
        }
    }
    CHECKED_CPX_CALL(CPXgettime, env, &timestamp2);
    std::cout << "time: " << timestamp2 - timestamp1;
    std::cout.flush();

    // add request constraints (destinations) [ forall j, sum{i} x_ij >= R_j ]
    std::cout << "\ncreating constraints destinations...";
    std::cout.flush();
    CHECKED_CPX_CALL(CPXgettime, env, &timestamp1);
    for (int j = 0; j < J; j++) {
        if ((j % 100) == 0) {
            std::cout << j << " ";
            std::cout.flush();
        }
        std::vector<int> idx;
        std::vector<double> coef;
        char sense = 'G';
        for (int i = 0; i < I; i++) {
            if (map_x[i][j] >= 0) {
                idx.push_back(map_x[i][j]);  // corresponds to variable x_ij
                coef.push_back(1.0);         // corresponds to variable x_ij
            }
        }
        int matbeg = 0;
        CHECKED_CPX_CALL(CPXaddrows, env, lp, 0, 1, idx.size(), &R[j], &sense, &matbeg, &idx[0], &coef[0], NULL, NULL);
        /// status =      CPXaddrows (        env, lp, colcnt, rowcnt, nzcnt     , rhs  , sense , rmatbeg, rmatind, rmatval , newcolname, newrowname);
    }
    CHECKED_CPX_CALL(CPXgettime, env, &timestamp2);
    std::cout << "time: " << timestamp2 - timestamp1;
    std::cout.flush();

    // add capacity constraints (origin) [ forall i, sum{j} x_ij <= D_j ] ///TODO-done
    std::cout << "\ncreating constraints origins...";
    std::cout.flush();
    CHECKED_CPX_CALL(CPXgettime, env, &timestamp1);
    for (int i = 0; i < I; i++) {
        if ((i % 100) == 0) {
            std::cout << i << " ";
            std::cout.flush();
        }
        std::vector<int> idx;
        std::vector<double> coef;
        char sense = 'L';
        for (int j = 0; j < J; j++) {
            if (map_x[i][j] >= 0) {
                idx.push_back(map_x[i][j]);  // corresponds to variable x_ij
                coef.push_back(1.0);         // corresponds to variable x_ij
            }
        }
        int matbeg = 0;
        CHECKED_CPX_CALL(CPXaddrows, env, lp, 0, 1, idx.size(), &D[i], &sense, &matbeg, &idx[0], &coef[0], 0, 0);
    }
    CHECKED_CPX_CALL(CPXgettime, env, &timestamp2);
    std::cout << "time: " << timestamp2 - timestamp1;
    std::cout.flush();
}

int main(int argc, char const* argv[]) {
    try {
        std::cout << "Number of origins is      " << I << std::endl;
        std::cout << "Number of destinations is " << J << std::endl;
        float costThreshold;
        std::cout << "Consider links with unit cost up to [0..10]? ";
        std::cin >> costThreshold;

        DECL_ENV(env);
        double timestamp1, timestamp2;
        std::cout << "\ncreating data...";
        std::cout.flush();
        srand(2);
        CHECKED_CPX_CALL(CPXgettime, env, &timestamp1);
        std::string tmpstring;
        double tot_D = 0.0;
        for (int i = 0; i < I; ++i) {
            tmpstring = std::to_string(i);
            nameI[i] = *tmpstring.c_str();
            D[i] = rand() % 100;
            tot_D += D[i];
        }
        double tot_R = 0.0;
        for (int j = 0; j < J; ++j) {
            tmpstring = std::to_string(j);
            nameJ[j] = *tmpstring.c_str();
            R[j] = rand() % 100;
            tot_R += R[j];
        }
        if (tot_R > tot_D) {
            D[0] += (tot_R - tot_D);
        }  // for the sake of feasibility
        for (int i = 0; i < I; ++i) {
            for (int j = 0; j < J; ++j) {
                C[i * J + j] = rand() % 10 + 0.5;
            }
        }
        CHECKED_CPX_CALL(CPXgettime, env, &timestamp2);
        std::cout << "time: " << timestamp2 - timestamp1;
        std::cout.flush();

        std::cout << "\ncreating model environment...";
        std::cout.flush();
        // init
        DECL_PROB(env, lp);
        // setup LP
        CHECKED_CPX_CALL(CPXgettime, env, &timestamp1);
        setupLP(env, lp, costThreshold);
        CHECKED_CPX_CALL(CPXgettime, env, &timestamp2);
        std::cout << "\nTOTAL time to create the model: " << timestamp2 - timestamp1 << endl;

        int n = CPXgetnumcols(env, lp);
        std::cout << n << " variables: type to continue";
        char any;
        std::cin >> any;
        // optimize
        CHECKED_CPX_CALL(CPXgettime, env, &timestamp1);
        CHECKED_CPX_CALL(CPXchgprobtype, env, lp, CPXPROB_LP);
        CHECKED_CPX_CALL(CPXsetintparam, env, CPX_PARAM_SCRIND, CPX_ON);
        CHECKED_CPX_CALL(CPXlpopt, env, lp);
        CHECKED_CPX_CALL(CPXgettime, env, &timestamp2);
        std::cout << "\nSOLVER time: " << timestamp2 - timestamp1;
        std::cout.flush();
        // print
        double objval;
        CHECKED_CPX_CALL(CPXgetobjval, env, lp, &objval);
        std::cout << "\nObjval: " << objval << std::endl;
        // CHECKED_CPX_CALL( CPXsolwrite, env, lp, "ironrods.sol" );
        //  free
        CPXfreeprob(env, &lp);
        CPXcloseCPLEX(&env);

        char luino;
        std::cin >> luino;
    } catch (std::exception& e) {
        std::cout << ">>>EXCEPTION: " << e.what() << std::endl;
    }
    return 0;
}
