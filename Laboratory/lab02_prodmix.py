from docplex.mp.model import Model

model_name = "prodmix_farmer"
var_type = "cont"
J = ["tom", "pot"]
I = ["land", "tseeds", "ptub", "fert"]
D = {"land": 11, "tseeds": 70, "ptub": 18, "fert": 145}
P = {"tom": 6000, "pot": 7000}
Q = {
    ("land", "tom"): 1,
    ("land", "pot"): 1,
    ("tseeds", "tom"): 7,
    ("tseeds", "pot"): 0,
    ("ptub", "tom"): 0,
    ("ptub", "pot"): 3,
    ("fert", "tom"): 10,
    ("fert", "pot"): 20,
}

# model_name = "prodmix_perfumes"
# var_type = "cont"
# J = ["one", "two"]
# I = ["rose", "lily", "violet"]
# D = {"rose": 27, "lily": 21, "violet": 9}
# P = {"one": 130, "two": 100}
# Q = {
#     ("rose", "one"): 1.5,
#     ("rose", "two"): 1,
#     ("lily", "one"): 1,
#     ("lily", "two"): 1,
#     ("violet", "one"): 0.3,
#     ("violet", "two"): 0.5,
# }


m = Model(name=model_name)

# Create the variables and store them into a dictionary
if var_type == "cont":
    x = m.continuous_var_dict(J, name="x")
elif var_type == "int":
    x = m.integer_var_dict(J, name="x")
elif var_type == "bin":
    x = m.binary_var_dict(J, name="x")
else:
    print("ERROR: wrong var type")
    exit(1)

# Add the constraints
m.add_constraints(m.sum(Q[i, j] * x[j] for j in J) <= D[i] for i in I)

# Set the objective function
m.maximize(m.sum(P[j] * x[j] for j in J))

# Write the lp
m.export_as_lp(f"{model_name}.lp")

# Solve and print solution
if m.solve(log_output=False):
    m.print_solution()
else:
    print(m.get_solve_status())
