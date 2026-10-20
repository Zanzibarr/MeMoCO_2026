from docplex.mp.model import Model
import json

with open("data/lab03_emergency_direct.json", "r") as f:
    data = json.load(f)

model_name = data["modelname"]
domain = data["decision_domain"]
L = data["zoneI"]
D = data["distance_matrix"]
T = data["distance_threshold"]

m = Model(model_name)

if domain == "binary":
    x = m.binary_var_dict(keys=L, name="open")
else:
    print("Unexpected domain")
    exit(1)

m.minimize(m.sum(x[i] for i in L))

m.add_constraints(
    m.sum(x[i] for i in L if D[L.index(j)][L.index(i)] <= T) >= 1 for j in L
)

m.export_as_lp(path=f"{model_name}.lp")

if m.solve():
    m.print_solution()
else:
    print(m.get_solve_status())
