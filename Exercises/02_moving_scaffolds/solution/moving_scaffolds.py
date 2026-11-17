from docplex.mp.model import Model

"""
Moving scaffolds (same problem as the C++ versions in this folder).

I    : closing sites (origins), D_i rods available in i
J    : new sites (destinations), R_j rods required in j
C_ij : cost of moving one rod from i to j (here in euros: the data are given in eurocents)
K    : truck capacity, F: fixed cost per truck, N: available trucks,
L    : cost of renting one additional truck

== Variables (basic version)
x_ij : number of rods moved from i to j     (integer)
y_ij : 1 if a truck is used on link (i,j)   (binary)
z    : 1 if the additional truck is rented  (binary)

== Model (basic version)
min sum_{i,j} C_ij x_ij + F sum_{i,j} y_ij + (L - F) z
s.t.
sum_{i in I} x_ij >= R_j            forall j in J
sum_{j in J} x_ij <= D_i            forall i in I
x_ij <= K y_ij                      forall i, j
sum_{i,j} y_ij <= N + z
sum_{i in X} y_id <= 1              forall (d, X) in exclusiveSupply  (d served by at most one of X)

Set `version` below to choose the model:
  "basic"    : the model above
  "variant1" : more trucks per link (w_ij integer), any number of extra trucks (z integer)
  "variant2" : variant1 + fixed cost A_i for using origin i (v_i binary)

Differences with the C++ versions in this folder:
- costs: here c_matrix is in eurocents (C_ij = 0.01 * c_matrix), while C++ uses
  the same numbers as euros, so the objective values differ (1165 here, basic version);
- the exclusiveSupply constraint (y_A2 + y_B2 <= 1) is in the basic version here,
  in C++ only in moving_scaffolds.cpp (not in the *_map* files);
- variant1/variant2 exist only here; EXT1/EXT2 exist only in C++.
"""

# MOVING SCAFFOLDS

# data: sets
I = ["A", "B", "C"]  # set of closing sites
J = [1, 2, 3]  # set of new sites

# data: problem parameteres
D = {}  # numer of rods available at each origin
d_list = [7000, 6000, 4000]  # ORDERED list of the rods at each origin
for i in range(len(I)):  # put in a dictionary for ease of reference
    D[I[i]] = d_list[i]
R = {}  # number of rods required at each destination
r_list = [8000, 5000, 4000]  # ORDERED list of the rods request at each destination
for j in range(len(J)):
    R[J[j]] = r_list[j]
C = {}  # transport cost of a unit from origin i to destination j (in euro)
c_matrix = [[9, 6, 5], [7, 4, 9], [4, 6, 3]]  # in eurocents
for i in range(len(I)):
    for j in range(len(J)):
        C[(I[i], J[j])] = 0.01 * c_matrix[i][j]
N = 4  # number of available trucks
F = 50  # fixed cost for each truck (euros)
K = 10000  # trucks capacity
L = 65  # fixed cost for the rent of an additional truck (euros)
exclusiveSupply = [
    (2, ["A", "B"])
]  # we generalize with a set of pairs (d,X), where a is d destination and X is a list of nutually exclusive alternatives

# MODEL CREATION
version = "basic"  # "basic" or "variant1" or "variant2"
if version == "basic":
    m = Model(name="moving scaffolds basic")
    x = m.integer_var_matrix(keys1=I, keys2=J, lb=0, ub=None, name="x")
    y = m.binary_var_matrix(keys1=I, keys2=J, name="y")
    z = m.binary_var(name="z")

    m.minimize(
        m.sum(C[(i, j)] * x[(i, j)] + F * y[(i, j)] for i in I for j in J) + (L - F) * z
    )

    m.add_constraints(m.sum(x[(i, j)] for i in I) >= R[j] for j in J)
    m.add_constraints(m.sum(x[(i, j)] for j in J) <= D[i] for i in I)
    m.add_constraints(x[(i, j)] <= K * y[(i, j)] for i in I for j in J)
    m.add_constraint(m.sum(y[(i, j)] for i in I for j in J) <= N + z)
    m.add_constraints(sum(y[i, d] for i in X) <= 1 for (d, X) in exclusiveSupply)

elif version == "variant1":
    m = Model(name="moving scaffolds v1")
    x = m.integer_var_matrix(keys1=I, keys2=J, lb=0, ub=None, name="x")
    w = m.integer_var_matrix(keys1=I, keys2=J, lb=0, ub=None, name="w")  # new var
    y = m.binary_var_matrix(keys1=I, keys2=J, name="y")
    z = m.integer_var(lb=0, ub=None, name="z")  # domain change

    m.minimize(
        m.sum(C[(i, j)] * x[(i, j)] + F * w[(i, j)] for i in I for j in J) + (L - F) * z
    )

    m.add_constraints(m.sum(x[(i, j)] for i in I) >= R[j] for j in J)
    m.add_constraints(m.sum(x[(i, j)] for j in J) <= D[i] for i in I)
    m.add_constraints(x[(i, j)] <= K * w[(i, j)] for i in I for j in J)  # new var
    m.add_constraint(m.sum(w[(i, j)] for i in I for j in J) <= N + z)  # new var
    bigM = D  # D[i] is an upper bound on x[(i,j)], for every j
    m.add_constraints(
        x[(i, j)] <= bigM[i] * y[(i, j)] for i in I for j in J
    )  # new bigM
    m.add_constraints(sum(y[i, d] for i in X) <= 1 for (d, X) in exclusiveSupply)

elif version == "variant2":
    A = {}  # fixed costs for moving something from origin i in I
    a_list = [100, 150, 175]  # ORDERED list of the fixed costs at each origin
    for i in range(len(I)):
        A[I[i]] = a_list[i]
    m = Model(name="moving scaffolds v2")
    x = m.integer_var_matrix(keys1=I, keys2=J, lb=0, ub=None, name="x")
    w = m.integer_var_matrix(keys1=I, keys2=J, lb=0, ub=None, name="w")
    y = m.binary_var_matrix(keys1=I, keys2=J, name="y")
    z = m.integer_var(lb=0, ub=None, name="z")
    v = m.binary_var_dict(keys=I, name="v")  # new var

    m.minimize(
        m.sum(C[(i, j)] * x[(i, j)] + F * w[(i, j)] for i in I for j in J)
        + (L - F) * z
        + sum(A[i] * v[i] for i in I)
    )  # new component

    m.add_constraints(m.sum(x[(i, j)] for i in I) >= R[j] for j in J)
    m.add_constraints(
        m.sum(x[(i, j)] for j in J) <= D[i] * v[i] for i in I
    )  # also sets new var
    m.add_constraints(x[(i, j)] <= K * w[(i, j)] for i in I for j in J)
    m.add_constraint(m.sum(w[(i, j)] for i in I for j in J) <= N + z)
    bigM = D  # D[i] is an upper bound on x[(i,j)], for every j
    m.add_constraints(x[(i, j)] <= bigM[i] * y[(i, j)] for i in I for j in J)
    m.add_constraints(sum(y[i, d] for i in X) <= 1 for (d, X) in exclusiveSupply)

m.print_information()  # display information on the model
m.export_as_lp(path=".")

if m.solve():
    m.print_solution()  # display information on the solution
    m.solution.export(
        "moving_scaffolds_" + version + "_solution.json"
    )  # exports solution in a json file
else:
    print(m.get_solve_status())
