import random as r
from docplex.mp.model import Model

# DATA: sets and parameters
n_op = int(input("Number of operations? "))
n_room = int(input("Number of rooms? "))
n_day = 7
w = [3 + r.randint(0, 5) for i in range(n_op)]  # operation duration
withinday = [
    r.randint(0, n_day - 1) if r.random() <= 0.5 else n_day for i in range(n_op)
]  # n_day means optional ("white")
Wroom = [10 + r.randint(0, 10) for room in range(n_room)]  # room capacity

# model parameters
I = range(n_op)  # set of operations
J = range(n_room * n_day)  # set of rooms in each day (day rooms)
W = [Wroom[int(j / n_day)] for j in J]  # day-room capacity

comp_pairs = [
    (i, j) for i in I for j in J if j % n_day <= withinday[i]
]  # operation-to-room compatibility


# MODEL
print("Creating model...")
m = Model(name="operating rooms")

x = m.binary_var_dict(comp_pairs, name="x")

m.maximize(m.sum(x[ij] for ij in comp_pairs))

for j in J:
    m.add_constraint(
        m.sum(w[ij[0]] * x[ij] for ij in comp_pairs if ij[1] == j) <= W[j],
        ctname="capacity of room %d in day %d" % (int(j / n_day), j % n_day),
    )

for i in I:
    if withinday[i] < n_day:
        m.add_constraint(
            m.sum(x[ij] for ij in comp_pairs if ij[0] == i) == 1,
            ctname="assign mandatory operation %d" % i,
        )
    else:
        m.add_constraint(
            m.sum(x[ij] for ij in comp_pairs if ij[0] == i) <= 1,
            ctname="assign optional operation %d" % i,
        )

m.print_information()
m.export_as_lp(path=".")

m.parameters.timelimit = 30  # stop after 30 seconds

print("solving model...")
if m.solve():
    m.print_solution()  # display information on the solution
    print(m.get_solve_status())
    print(
        "O.F. value is ",
        m.objective_value,
        " (cannot be more than ",
        m.solve_details.best_bound,
        ", i.e., ",
        "{0:.2f}".format(100 * m.solve_details.mip_relative_gap),
        "% more)",
    )
else:
    print("no solution found:", m.get_solve_status())
print("solving time:", m.solve_details.time)
