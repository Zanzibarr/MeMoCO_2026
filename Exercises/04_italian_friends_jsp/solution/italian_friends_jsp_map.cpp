/**
 * @file italian_friends_jsp_map.cpp
 * @use of "maps" to make the access to variable indexes easier
 *
 * Since we do not create variables x_ijk when i==j, computing
 * the index of a variable x for a given triplet (i,j,k) may be not
 * straightforward (the usual formula "x_init + i*(I*I) + j*I + k"
 * will not work, since we have "holes" in the linearized 3-dim matrix
 * corresponding to variables x in the unique Cplex variable array).
 * For this reason, we can use a "map" to associate
 * to each triplet (i,j,k) the index of the corresponding x variable.
 * The map is initialized during the definition of the variables
 * (calls to "CPXnewcols") and used to get indexes during constraints
 * definition (calls to "CPXaddrows").
 * In the code below, a map for x variables is implemented as a
 * three-dimensional "vector" (map_x), of course you may use your favorite
 * data structure as an alternative, for example to allow using the name
 * of set elements as keys, or to allow retrieving the key corresponding
 * to a given variable index (for example to obtain a meaningful output
 * for the value of x in in the solution). You will find further comments
 * in the code.
 * The use of maps may be conveniently generalized to all variables,
 * to make access to their indexes easier.
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
const int I = 4;
const int K = 4;
const char nameI[I] = {'A', 'B', 'C', 'D'};
const char nameK[K] = {'R', 'M', 'S', 'G'};

const double R[I] = {0.0, 15.0, 15.0, 60.0};
// reading time D(friend, newspaper), LINEARIZED as D[i*K + k]
//                          R     M    S     G
const double D[I * K] = {60.0, 2.0, 30.0, 5.0,    // A
                         25.0, 3.0, 75.0, 10.0,   // B
                         10.0, 5.0, 15.0, 30.0,   // C
                         1.0,  1.0, 1.0,  90.0};  // D
// reading order: S[i*K + l] = index of the l-th newspaper read by friend i (sigma(i,l), 0-based)
const int S[I * K] = {0, 2, 1, 3,   // A: R S M G
                      2, 1, 0, 3,   // B: S M R G
                      1, 2, 0, 3,   // C: M S R G
                      3, 0, 2, 1};  // D: G R S M

double M;  // big M, initialized in setupLP

/*MAP FOR X VARS: declare map*/
vector<vector<vector<int> > > map_x;  // x_ijk ---> map_x[i][j][k]

const int NAME_SIZE = 512;
char name[NAME_SIZE];

void setupLP(CEnv env, Prob lp, int& numVars) {
    // determine a feasible value for M [ max_{i in I} R_i + sum_{i,k} D_{i,k} ]
    M = 0;
    for (int i = 0; i < I; ++i)
        if (R[i] > M) M = R[i];
    for (int i = 0; i < I; i++) {
        for (int k = 0; k < K; k++) {
            M += D[i * K + k];
        }
    }

    /*MAP FOR X VARS: initial memory allocation for map vector*/
    map_x.resize(I);
    for (int i = 0; i < I; ++i) {
        map_x[i].resize(I);
        for (int j = 0; j < I; ++j) {
            map_x[i][j].resize(K);
            for (int k = 0; k < K; ++k) {
                map_x[i][j][k] = -1;
            }
        }
    }

    const int h_init = 0;  // first index for h vars
    // add h vars
    for (int i = 0; i < I; i++) {
        for (int k = 0; k < K; k++) {
            char htype = 'C';
            double obj = 0.0;
            double lb = 0.0;
            double ub = CPX_INFBOUND;
            snprintf(name, NAME_SIZE, "h_%c_%c", nameI[i], nameK[k]);
            char* hname = (char*)(&name[0]);
            CHECKED_CPX_CALL(CPXnewcols, env, lp, 1, &obj, &lb, &ub, &htype, &hname);
        }
    }

    // add x vars
    const int x_init = CPXgetnumcols(env, lp);  // first index for x vars

    /*MAP FOR X VARS: initialize the position of the x var that is being created*/
    int current_var_position = x_init;

    for (int k = 0; k < K; k++) {
        for (int i = 0; i < I; i++) {
            for (int j = 0; j < I; j++) {
                if (i == j) continue;

                // TODO...DONE
                char xtype = 'B';
                double obj = 0.0;
                double lb = 0.0;
                double ub = 1.0;
                snprintf(name, NAME_SIZE, "x_%c_%c_%c", nameI[i], nameI[j], nameK[k]);
                char* xname = (char*)(&name[0]);
                CHECKED_CPX_CALL(CPXnewcols, env, lp, 1, &obj, &lb, &ub, &xtype, &xname);

                /*MAP FOR X VARS: store the index of the current x_ijk*/
                map_x[i][j][k] = current_var_position;
                ++current_var_position;
            }
        }
    }
    // add y var
    const int yIdx = CPXgetnumcols(env, lp);  // index of y var
    {
        char ytype = 'C';
        double lb = 0.0;
        double ub = CPX_INFBOUND;
        double obj = 1.0;
        snprintf(name, NAME_SIZE, "y");
        char* yname = (char*)(&name[0]);
        CHECKED_CPX_CALL(CPXnewcols, env, lp, 1, &obj, &lb, &ub, &ytype, &yname);
    }
    numVars = CPXgetnumcols(env, lp);

    // add finish-time constraints [ y >= h_{i S[i,|K|]} + D_{i S[i,|K|]} ]
    for (int i = 0; i < I; i++) {
        std::vector<int> idx(2);
        idx[0] = yIdx;
        idx[1] = h_init + i * K + S[i * K + (K - 1)];  // sigma(i,last)
        std::vector<double> coef(2);
        coef[0] = 1.0;
        coef[1] = -1.0;
        char sense = 'G';
        int matbeg = 0;
        CHECKED_CPX_CALL(CPXaddrows, env, lp, 0, 1, idx.size(), &D[i * K + S[i * K + (K - 1)]], &sense, &matbeg, &idx[0], &coef[0], 0, 0);
    }

    // add wake-up constraints [ h_{i S[i,|K|]} >= R_i ]
    // TODO...done
    for (int i = 0; i < I; i++) {
        int idx = h_init + i * K + S[i * K + 0];  // h[ i , sigma(i,first) ]
        double coef = 1.0;
        char sense = 'G';
        int matbeg = 0;
        CHECKED_CPX_CALL(CPXaddrows, env, lp, 0, 1, 1, &R[i], &sense, &matbeg, &idx, &coef, 0, 0);
    }

    // add non-overlapping constraints between consecutive newspapers [ h_{i S[i,l]} >= h_{i S[i,l-1]} + D_{i S[i,l-1]} ]
    for (int i = 0; i < I; i++) {
        for (int l = 1; l < K; l++) {
            // TODO...done
            std::vector<int> idx(2);
            idx[0] = h_init + i * K + S[i * K + l];        // h[ i , sigma(i,l) ]
            idx[1] = h_init + i * K + S[i * K + (l - 1)];  // h[ i , sigma(i,l-1) ]
            std::vector<double> coef(2);
            coef[0] = 1.0;
            coef[1] = -1.0;
            char sense = 'G';
            int matbeg = 0;
            CHECKED_CPX_CALL(CPXaddrows, env, lp, 0, 1, idx.size(), &D[i * K + S[i * K + (l - 1)]], &sense, &matbeg, &idx[0], &coef[0], 0, 0);
        }
    }

    // add non-overlapping constraints between people on the same newspaper
    /**MAP FOR X VARS: we will take directly the index, without case-specific tricks**/
    /*this is not needed!	int xIdx = x_init;
     */
    for (int k = 0; k < K; k++) {
        for (int i = 0; i < I; i++) {
            for (int j = 0; j < I; j++) {
                if (i == j) continue;

                // 1 [ h_{i k} >= h_{j k]} + D_{j k]} - M x_ijk ]
                std::vector<int> idx(3);
                idx[0] = h_init + i * K + k;
                idx[1] = h_init + j * K + k;
                idx[2] = map_x[i][j][k];  // xIdx;
                /**MAP FOR X VARS: take directly the index, without case-specific tricks**/
                std::vector<double> coef(3);
                coef[0] = 1.0;
                coef[1] = -1.0;
                coef[2] = M;
                char sense = 'G';
                int matbeg = 0;
                CHECKED_CPX_CALL(CPXaddrows, env, lp, 0, 1, idx.size(), &D[j * K + k], &sense, &matbeg, &idx[0], &coef[0], 0, 0);

                // 2 [ h_{j k} >= h_{i k]} + D_{i k]} - M (1 -x_ijk) ]
                // TODO...done
                idx[0] = h_init + j * K + k;
                idx[1] = h_init + i * K + k;
                idx[2] = map_x[i][j][k];  // xIdx;
                /**MAP FOR X VARS: take directly the index, without case-specific tricks**/
                coef[0] = 1.0;
                coef[1] = -1.0;
                coef[2] = -M;
                sense = 'G';
                matbeg = 0;
                double rhs = D[i * K + k] - M;
                CHECKED_CPX_CALL(CPXaddrows, env, lp, 0, 1, idx.size(), &rhs, &sense, &matbeg, &idx[0], &coef[0], 0, 0);

                /**MAP FOR X VARS: we take directly the index, we do not need case-specific tricks**/
                /*this is not needed!	xIdx++; // we have as many constraints as many x variables
                        // and the nested loops for constraints adding
                        // follow the same order as the one for x variable adding
                 */
            }
        }
    }

    // print (debug)
    CHECKED_CPX_CALL(CPXwriteprob, env, lp, "italian_friends_jsp_map.lp", 0);
}

int main(int argc, char const* argv[]) {
    // TODO...done
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
        cout << "check num var: " << n << " == " << numVars << endl;
        if (n != numVars) {
            throw std::runtime_error(std::string(__FILE__) + ":" + STRINGIZE(__LINE__) + ": " + "different number of variables");
        }
        std::vector<double> varVals;
        varVals.resize(n);
        CHECKED_CPX_CALL(CPXgetx, env, lp, &varVals[0], 0, n - 1);
        for (int i = 0; i < n; ++i) {
            std::cout << "var in position " << i << " : " << varVals[i] << std::endl;
            /**MAP FOR X VARS: the use of advanced data structures for maps may help
            retrieving the triplet (i,j,k) or any "meaningful" identifier related to
            "position" (cplex index), as to obtain a more meaningful output without using
            the tricky "CPXgetcolname" API function**/
        }
        CHECKED_CPX_CALL(CPXsolwrite, env, lp, "italian_friends_jsp_map.sol");
        // free
        CPXfreeprob(env, &lp);
        CPXcloseCPLEX(&env);
    } catch (std::exception& e) {
        std::cout << ">>>EXCEPTION: " << e.what() << std::endl;
    }

    return 0;
}
