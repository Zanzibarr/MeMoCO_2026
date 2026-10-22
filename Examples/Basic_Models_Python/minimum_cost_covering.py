from docplex.mp.model import Model

"""
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
x_i in R_+/Z_+/{0,1} (depends on the type of resource)       forall i in I
"""

# Model creation
model = Model(name="cost_covering")

# Data (with the diet example)
I = range(3)  # vegetables, meat and fruit
J = range(3)  # proteins, iron and calcium
C = [4, 10, 7]  # C[i] = unit cost of resource i in I
R = [20, 30, 10]  # R[j] = requested amount of j in J
A = [
    [5, 6, 5],
    [15, 10, 3],
    [4, 5, 12],
]  # A[i][j] = amount of request j in J satisfied by one unit of resource i in I

# Variables creation
x = model.continuous_var_list(len(I), lb=0, name="x")

# Constraints
for j in J:
    model.add_constraint(
        model.sum(A[i][j] * x[i] for i in I) >= R[j], ctname=f"request_{j}"
    )

# Objective function
model.minimize(model.sum(C[i] * x[i] for i in I))

# Optimization
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
