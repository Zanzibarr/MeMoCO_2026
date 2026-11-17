from docplex.mp.model import Model
import ast
import json
import sys

"""
Data-independent minimum cost covering model: the same code solves different
instances, read from a JSON file.

    python minimum_cost_covering.py                                 # default: diet
    python minimum_cost_covering.py ../../Data/mcc_emergency.json   # emergency location

I   : resources
J   : requests
C_i : unit cost of resource i in I
R_j : requested amount of j in J
A_ij: amount of request j in J satisfied by one unit of resource i in I

== Variables
x_i : amount of resource i in I

== Model
min sum_{i in I} C_i x_i
s.t.
sum_{i in I} A_ij x_i >= R_j     forall j in J
x_i in R_+/Z_+/{0,1} (given by "decision_domain" in the JSON file)   forall i in I

With the emergency location data, x_i = 1 if an emergency unit is placed in
location i, and A_ij = 1 if location i is close enough to serve zone j: every
zone must be covered by at least one unit.
"""

# Data (read from the JSON file given on the command line, or the default one)
path = sys.argv[1] if len(sys.argv) > 1 else "../../Data/mcc_diet.json"
with open(path, "r") as file:
    data = json.load(file)

model_name = data["modelname"]
I = data["resourceSetI"]  # list of resource names
J = data["requestSetJ"]  # list of request names
C = data["costC"]  # C[i] = unit cost of resource i in I
R = data["requestR"]  # R[j] = requested amount of j in J
# JSON keys are always strings: "('V','p')" must become the tuple ('V', 'p')
A = {ast.literal_eval(k): v for k, v in data["contributionA"].items()}  # A[i, j]
decision_domain = data["decision_domain"]  # "continuous", "discrete" or "binary"

# Model creation
model = Model(name=f"cost_covering: {model_name}")

# Variables creation (one per resource, indexed by its name; the domain depends on the data)
if decision_domain == "continuous":
    x = model.continuous_var_dict(I, lb=0, name="x")
elif decision_domain == "discrete":
    x = model.integer_var_dict(I, lb=0, name="x")
elif decision_domain == "binary":
    x = model.binary_var_dict(I, name="x")
else:
    print(f"ERROR: unknown decision domain '{decision_domain}'")
    sys.exit(1)

# Constraints
for j in J:
    model.add_constraint(
        model.sum(A[i, j] * x[i] for i in I) >= R[j], ctname=f"request_{j}"
    )

# Objective function
model.minimize(model.sum(C[i] * x[i] for i in I))

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
