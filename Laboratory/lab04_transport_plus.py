from docplex.mp.model import Model
import json
import sys

with open("data/lab04_transport_plus.json", "r") as file:
    data = json.load(file)

model_name = data["modelname"]
I = data["I"]
J = data["J"]
o_list = data["o_list"]
d_list = data["d_list"]
c_matrix = data["c_matrix"]
decision_domain = data["decision_domain"]
cost_threshold_percent = data["cost_threshold_percent"]
LowCost = data["LowCost"]
LowCostMinOnLink = data["LowCostMinOnLink"]
SpecialDestination = data["SpecialDestination"]
MinToSpecialDestination = data["MinToSpecialDestination"]
SpecialOrigin = data["SpecialOrigin"]
SignificantNumber = data["SignificantNumber"]
SignificantFraction = data["SignificantFraction"]

O = {I[i]: o_list[i] for i in range(len(I))}
D = {J[j]: d_list[j] for j in range(len(J))}
C = {(I[i], J[j]): c_matrix[i][j] for i in range(len(I)) for j in range(len(J))}

m = Model(name=model_name)

# define the set of "non expensive" accepted links
ActiveODpairs = [
    (i, j)
    for i in I
    for j in J
    if C[(i, j)] <= cost_threshold_percent * max(C[(o, d)] for o in O for d in D)
]

# define variables only for accepted links (cannot use "matrix")
if decision_domain == "discrete":
    x = m.integer_var_dict(keys=ActiveODpairs, lb=0, ub=None, name="x")
else:
    x = m.continuous_var_dict(keys=ActiveODpairs, lb=0, ub=None, name="x")

m.minimize(m.sum(C[(i, j)] * x[(i, j)] for (i, j) in ActiveODpairs))

# constraints: basic
m.add_constraints(
    m.sum(x[(i, j)] for i in I if (i, j) in ActiveODpairs) >= D[j] for j in J
)
m.add_constraints(
    m.sum(x[(ii, jj)] for (ii, jj) in ActiveODpairs if ii == i) <= O[i] for i in I
)

# constraint: additional 1 (see slides)
m.add_constraints(
    x[(i, j)] >= LowCostMinOnLink for (i, j) in ActiveODpairs if C[(i, j)] <= LowCost
)

# constraint: additional 2 (see slides)
m.add_constraints(
    x[(i, SpecialDestination)] >= MinToSpecialDestination
    for i in I
    if i != SpecialOrigin and (i, SpecialDestination) in ActiveODpairs
)
# or, equivalently
# m.add_constraints( x[(i,j)] >= MinToSpecialDestination for (i,j) in ActiveODpairs if i != SpecialOrigin and j == SpecialDestination )

# constraint: additional 3 (see slides)
y = m.binary_var_dict(ActiveODpairs, name="y")
for j in J:
    m.add_constraint(
        m.sum(y[od] for od in ActiveODpairs if od[1] == j) >= SignificantNumber
    )
m.add_constraints(
    x[od] >= SignificantFraction * D[od[1]] * y[od] for od in ActiveODpairs
)  # "activate" y variables

m.export_as_lp(path=f"{model_name}_plus.lp")

# SOLVE MODEL AND REPORT SOLUTION
if m.solve():
    m.print_solution()
else:
    print(m.get_solve_status())
