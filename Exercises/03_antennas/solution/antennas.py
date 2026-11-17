from docplex.mp.model import Model

"""
Antennas location.

I    : candidate sites for antennas
J    : areas to cover
S_ij : signal intensity from an antenna in i to area j
T    : minimum signal intensity: area j is reached by i if S_ij >= T
N    : maximum number of signals an area can receive without interference
M_j  : big M, |{i : S_ij >= T}| - N
bridge_pairs : (a, b) = an antenna in a needs an antenna in b

== Variables
x_i : 1 if an antenna is installed in i in I
z_j : 1 if area j in J is covered without interference

== Model
max sum_{j in J} z_j
s.t.
sum_{i : S_ij >= T} x_i >= z_j                  forall j in J  (covering)
sum_{i : S_ij >= T} x_i <= N + M_j (1 - z_j)    forall j in J  (no interference)
x_a <= x_b                                      forall (a, b) in bridge_pairs
x_i, z_j in {0,1}

Differences with the C++ version (antennas.cpp):
- here N = 2, in C++ N = 1;
- here M_j = |{i : S_ij >= T}| - N (tighter), in C++ M_j = |{i : S_ij >= T}|;
- the bridge constraint x_E <= x_D exists only here.
Hence the optimal values differ: 6 covered areas here, 5 in C++.
"""

# ANTENNAS LOCATION

# data: sets
I = ["A", "B", "C", "D", "E"]  # set of sites for antennas location
J = [1, 2, 3, 4, 5, 6]  # set of areas to cover
bridge_pairs = [("E", "D")]
# data: problem parameters
T = 18  # minimum signal required
N = 2  # maximum number of non-interfering signals
S = {}  # signal intensity from antenna i to area j
s_matrix = [
    [10, 20, 16, 25, 0, 10],
    [0, 12, 18, 23, 11, 6],
    [21, 8, 5, 6, 23, 19],
    [16, 15, 15, 8, 14, 18],
    [21, 13, 13, 17, 18, 22],
]
for i in range(len(I)):
    for j in range(len(J)):
        S[(I[i], J[j])] = s_matrix[i][j]

# model parameters
M = {
    j: len([i for i in I if S[(i, j)] >= T]) - N for j in J
}  # large enough "big-M" parameter
# M ={} # large enough parameter
# for j in J:
#    s_j = []
#    for item in s_matrix:
#        s_j.append(item[J.index(j)])
#    M[j] = len([s for s in s_j if s >= T])

# MODEL CREATION
m = Model(name="antennas")

x = m.binary_var_dict(keys=I, name="x")
z = m.binary_var_dict(keys=J, name="z")

m.maximize(m.sum(z[j] for j in J))

for j in J:
    m.add_constraint(
        m.sum(x[i] for i in I if S[(i, j)] >= T) >= z[j], ctname="covering zone %d" % j
    )
    m.add_constraint(
        m.sum(x[i] for i in I if S[(i, j)] >= T) <= N + M[j] * (1 - z[j]),
        ctname="non interfering in zone %d" % j,
    )

m.add_constraints(x[b[0]] <= x[b[1]] for b in bridge_pairs)

m.print_information()  # display information on the model
m.export_as_lp(basename="antennas", path=".")

if m.solve():
    m.print_solution()  # display information on the solution
    m.solution.export("antennas_solution.json")  # exports solution in a json file
else:
    print(m.get_solve_status())
