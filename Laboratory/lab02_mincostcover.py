from docplex.mp.model import Model

model_name = "mincostcover_diet"
var_type = "cont"
I = ["V", "M", "F"]
J = ["proteins", "iron", "calcium"]
C = {"V": 4, "M": 10, "F": 7}
R = {"proteins": 20, "iron": 30, "calcium": 10}
A = {
    ("V", "proteins"): 5,
    ("V", "iron"): 6,
    ("V", "calcium"): 5,
    ("M", "proteins"): 15,
    ("M", "iron"): 10,
    ("M", "calcium"): 3,
    ("F", "proteins"): 4,
    ("F", "iron"): 5,
    ("F", "calcium"): 12,
}

m = Model(name=model_name)

# Create the variables and store them into a dictionary
if var_type == "cont":
    x = m.continuous_var_dict(I, name="x")
elif var_type == "int":
    x = m.integer_var_dict(I, name="x")
elif var_type == "bin":
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
