from docplex.mp.model import Model

"""
Project scheduling with optional, alternative and interacting operations.

O        : operations, D_o duration of o, P_o predecessors of o
O_op     : optional operations (executed only if selected)
O_alt    : pairs of alternative operations: exactly one of each pair is executed
O_inc    : o -> (a, b): if both a and b are executed, o lasts d_inc_o more
M        : big M, sum of all durations (and increases)

== Variables
t_o    : completion time of operation o
y_o    : 1 if optional operation o is executed
w_(a,b): 1 if both a and b are executed
z      : project completion time (makespan)

== Model
min z
s.t.
z >= t_o                                     forall o in O
t_o >= t_k + D_o                             forall o not optional, k in P_o
t_o >= t_k + D_o - M (1 - y_o)               forall o in O_op, k in P_o  (only if executed)
t_o >= t_k + D_o + d_inc_o w_O_inc(o)        forall o in O_inc, k in P_o
t_o >= D_o                                   forall o with P_o empty
y_a + y_b = 1                                forall (a, b) in O_alt
y_a + y_b <= 1 + w_(a,b)                     forall (a, b) in O_inc values
"""

# SETS and PARAMETERS
O = ["A", "B", "C", "D", "E", "F", "G", "H", "I"]  # set of operations
P = {
    "A": [],
    "B": ["A"],
    "C": ["A"],
    "D": ["A"],
    "E": ["B", "C"],
    "F": ["E"],
    "G": ["E"],
    "H": ["D", "E", "G"],
    "I": ["F", "G"],
}  # operation : list of its predecessors

d_list = [2, 4, 2, 5, 3, 3, 2, 7, 4]  # ORDERED list of the duration of each operation

O_op = ["B", "C", "F", "G"]  # list of optional operations
O_alt = [("B", "C"), ("F", "G")]  # set of pairs of alternative operations
O_inc = {
    "I": ("C", "G")
}  # operation: pair of executed operations that increases the duration of the first one

d_inc = {"I": 2}  # duration increase of operations


# data preprocessing
D = {O[i]: d_list[i] for i in range(len(O))}  # durations in a handy dictionary
M = sum(D[i] for i in D) + sum(d_inc[i] for i in d_inc)


# MODEL

m = Model(name="project scheduling")

t = m.continuous_var_dict(keys=O, lb=0, name="t")
y = m.binary_var_dict(keys=O_op, name="y")
w = m.binary_var_dict(keys=O_inc.values(), name="w")
z = m.continuous_var(lb=0, name="z")

m.minimize(z)

for o in O:
    m.add_constraint(z >= t[o])

for o in O:
    if o in O_op:
        for k in P[o]:
            m.add_constraint(t[o] >= t[k] + D[o] - M * (1 - y[o]))
    elif o in O_inc.keys():
        for k in P[o]:
            m.add_constraint(t[o] >= t[k] + D[o] + d_inc[o] * w[O_inc[o]])
    else:
        for k in P[o]:
            m.add_constraint(t[o] >= t[k] + D[o])

m.add_constraints(t[o] >= D[o] for o in O if P[o] == [])

for i, j in O_alt:
    m.add_constraint(y[i] + y[j] == 1)

for i, j in O_inc.values():
    m.add_constraint(y[i] + y[j] <= 1 + w[(i, j)])

m.print_information()  # display information on the model
m.export_as_lp(path=".")

if m.solve():
    m.print_solution()  # display information on the solution
else:
    print(m.get_solve_status())
