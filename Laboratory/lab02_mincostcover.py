from docplex.mp.model import Model
from data.lab02_mincostcover_data import diet as data

model = Model(name=data.model_name)

# Create the variables and store them into a dictionary
if data.var_type == "cont":
    x = model.continuous_var_dict(data.I, name="x")
elif data.var_type == "int":
    x = model.integer_var_dict(data.I, name="x")
elif data.var_type == "bin":
    x = model.binary_var_dict(data.I, name="x")
else:
    print("ERROR: wrong var type")
    exit(1)

# Add the constraints
model.add_constraints(
    model.sum(data.A[i, j] * x[i] for i in data.I) >= data.R[j] for j in data.J
)

# Set the objective function
model.minimize(model.sum(data.C[i] * x[i] for i in data.I))

# Write the lp
model.export_as_lp(f"{data.model_name}.lp")

# Solve
solution = model.solve(log_output=False)

# Print solution
if solution is not None:
    print(f"===================================================")
    print(f"Objective function value: {solution.objective_value}")
    for v in model.iter_variables():
        print(v.name, solution[v])
    print(f"===================================================")
else:
    print(model.solve_details.status)
