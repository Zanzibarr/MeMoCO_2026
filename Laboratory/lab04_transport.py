from docplex.mp.model import Model
import json

with open("data/lab04_transport_basic.json", "r") as file:
    data = json.load(file)

model_name = data["modelname"]
I = data["I"]
J = data["J"]
o_list = data["o_list"]
d_list = data["d_list"]
c_matrix = data["c_matrix"]
decision_domain = data["decision_domain"]

O = {I[i]: o_list[i] for i in range(len(I))}
D = {J[j]: d_list[j] for j in range(len(J))}
C = {(I[i], J[j]): c_matrix[i][j] for i in range(len(I)) for j in range(len(J))}

m = Model(name=model_name)

if decision_domain == "discrete":
    x = m.integer_var_matrix(keys1=I, keys2=J, lb=0, ub=None, name="x")
else:
    x = m.continuous_var_matrix(keys1=I, keys2=J, lb=0, ub=None, name="x")

m.minimize(m.sum(C[i, j] * x[(i, j)] for i in I for j in J))

for j in J:
    m.add_constraint(m.sum(x[i, j] for i in I) >= D[j])

m.add_constraints(m.sum(x[(i, j)] for j in J) <= O[i] for i in I)

m.export_as_lp(path=f"{model_name}_basic.lp")

# SOLVE MODEL AND REPORT SOLUTION
if m.solve():
    m.print_solution()
else:
    print(m.get_solve_status())
