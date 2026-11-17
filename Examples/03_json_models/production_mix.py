from docplex.mp.model import Model
import ast
import json
import sys

"""
Data-independent production mix model: the same code solves different
instances, read from a JSON file.

    python production_mix.py                         # default: farmer 4.0
    python production_mix.py ../../Data/wines.json   # another instance

I   : resources
J   : products
D_i : availability of resource i in I
P_j : unit profit for product j in J
Q_ij: amount of resource i in I required for each unit of product j in J

== Variables
x_j : amount of product j in J

== Model
max sum_{j in J} P_j x_j
s.t.
sum_{j in J} Q_ij x_j <= D_i     forall i in I
x_j in R_+/Z_+/{0,1} (given by "decision_domain" in the JSON file)   forall j in J
"""

# Data (read from the JSON file given on the command line, or the default one)
path = sys.argv[1] if len(sys.argv) > 1 else "../../Data/farmer.json"
with open(path, "r") as file:
    data = json.load(file)

model_name = data["modelname"]
I = data["Resources_I"]  # list of resource names
J = data["Products_J"]  # list of product names
D = data["D"]  # D[i] = availability of resource i in I
P = data["P"]  # P[j] = profit of product j in J
# JSON keys are always strings: "('land','tomato')" must become the tuple ('land', 'tomato')
Q = {ast.literal_eval(k): v for k, v in data["Q"].items()}  # Q[i, j]
decision_domain = data["decision_domain"]  # "continuous", "discrete" or "binary"

# Model creation
model = Model(name=f"production_mix: {model_name}")

# Variables creation (one per product, indexed by its name; the domain depends on the data)
if decision_domain == "continuous":
    x = model.continuous_var_dict(J, lb=0, name="x")
elif decision_domain == "discrete":
    x = model.integer_var_dict(J, lb=0, name="x")
elif decision_domain == "binary":
    x = model.binary_var_dict(J, name="x")
else:
    print(f"ERROR: unknown decision domain '{decision_domain}'")
    sys.exit(1)

# Constraints
for i in I:
    model.add_constraint(
        model.sum(Q[i, j] * x[j] for j in J) <= D[i], ctname=f"resource_{i}"
    )

# Objective function
model.maximize(model.sum(P[j] * x[j] for j in J))

# Optimization
solution = model.solve(log_output=False)

# Print solution
if solution is not None:
    print(f"===================================================")
    print(f"Instance: {model_name}")
    print(f"Objective function value: {solution.objective_value}")
    for v in model.iter_variables():
        print(v.name, solution[v])
    print(f"===================================================")
else:
    print(model.solve_details.status)
