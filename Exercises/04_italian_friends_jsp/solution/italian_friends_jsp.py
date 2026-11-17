from docplex.mp.model import Model

"""
Job-shop scheduling: four friends (jobs I) read four newspapers (machines K).

D_ik       : minutes friend i spends reading newspaper k
O_(i,l)    : l-th newspaper read by friend i (l = 1..|K|)
R_i        : time friend i wakes up (release time, minutes after 8:30)
M          : big M, upper bound on the makespan

== Variables
h_ik  : time friend i starts reading newspaper k
x_ijk : 1 if i reads newspaper k before j (i != j)
y     : makespan

== Model
min y
s.t.
y >= h_(i,O(i,|K|)) + D_(i,O(i,|K|))                       forall i        (makespan)
h_(i,O(i,1)) >= R_i                                        forall i        (release time)
h_(i,O(i,l)) >= h_(i,O(i,l-1)) + D_(i,O(i,l-1))            forall i, l > 1 (reading order)
h_ik >= h_jk + D_jk - M x_ijk                              forall k, i != j  (one reader
h_jk >= h_ik + D_ik - M (1 - x_ijk)                        forall k, i != j   per newspaper)
"""

# SETS and PARAMETERS
I = ["A", "B", "C", "D"]  # set of jobs (the four friends)
K = ["R", "M", "S", "G"]  # set of machines (the newspapers)
d_list = [
    [60, 2, 30, 5],
    [25, 3, 75, 10],
    [10, 5, 15, 30],
    [1, 1, 1, 90],
]  # ORDERED list of processing times of job i on machine k (minutes spent by each friend with each newspaper)
# IMPORTANT NOTICE: for each job i, the processing times are ordered according to the order of K, the precedence order is taken care of by the following parameter
o_list = [
    ["R", "S", "M", "G"],
    ["S", "M", "R", "G"],
    ["M", "S", "R", "G"],
    ["G", "R", "S", "M"],
]  # order of the machines in which job i is processed (order in which the fiends read the newspapers)
r_list = [0, 15, 15, 60]  # release time of job i, in minutes after 8:30

# data preprocessing (transform ordered list into dictionaries, for ease of reference)
D = {(I[i], K[k]): d_list[i][k] for i in range(len(I)) for k in range(len(K))}
O = {(I[i], l + 1): o_list[i][l] for i in range(len(I)) for l in range(len(K))}
R = {I[i]: r_list[i] for i in range(len(I))}

# computed model parameters
M = sum(D[(i, k)] for i in I for k in K)  # ub on the makespan
N = len(K)  # number of machines


# MODEL
m = Model(name="job-shop scheduling")
y = m.continuous_var(lb=0, name="y")  # completion time
h = m.continuous_var_matrix(keys1=I, keys2=K, lb=0, name="h")  # start time of i on k
x = m.binary_var_dict(
    keys=[(i, j, k) for i in I for j in I for k in K if j != i], name="x"
)  # =1 if i precedes j on k

m.minimize(y)

for i in I:
    m.add_constraint(y >= h[(i, O[(i, N)])] + D[(i, O[(i, N)])])  # y is the makespan
    m.add_constraint(h[(i, O[(i, 1)])] >= R[i])  # job release times

for i in I:
    for l in range(2, N + 1):
        m.add_constraint(
            h[(i, O[(i, l)])] >= h[(i, O[(i, l - 1)])] + D[(i, O[(i, l - 1)])]
        )  # non-preemptive machine sequences per job

for k in K:
    for i in I:
        for j in I:
            if i != j:  # job sequence per machine (disjunctive constraints)
                m.add_constraint(h[(i, k)] >= h[(j, k)] + D[(j, k)] - M * x[(i, j, k)])
                m.add_constraint(
                    h[(j, k)] >= h[(i, k)] + D[(i, k)] - M * (1 - x[(i, j, k)])
                )

m.print_information()  # display information on the model
m.export_as_lp(path=".")

if m.solve():
    m.print_solution()  # display information on the solution
else:
    print(m.get_solve_status())
print("running time =", m.solve_details.time)
