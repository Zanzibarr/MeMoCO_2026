from docplex.mp.model import Model
import json
import ast

with open("data/lab03_mcc_diet.json", "r") as f:
    data = json.load(f)

model_name = data["modname"]
I = data["resSetI"]
J = data["reqSetJ"]
C = data["costC"]
R = data["reqR"]
A = {ast.literal_eval(k): v for k, v in data["contribA"].items()}
domain = data["decdom"]

m = Model(name=model_name)

# Create the variables and store them into a dictionary
if domain == "cont":
    x = m.continuous_var_dict(I, name="x")
elif domain == "int":
    x = m.integer_var_dict(I, name="x")
elif domain == "bin":
    x = m.binary_var_dict(I, name="x")
else:
    print("ERROR: wrong var type")
    exit(1)

# Add the constraints
m.add_constraints(m.sum(A[i, j] * x[i] for i in I) >= R[j] for j in J)

# Set the objective function
m.minimize(m.sum(C[i] * x[i] for i in I))

# Write the lp
m.export_as_lp(f"{model_name}.lp")

# Print solution
if m.solve():
    m.print_solution()
else:
    print(m.get_solve_status())
