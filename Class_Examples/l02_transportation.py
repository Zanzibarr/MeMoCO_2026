from docplex.mp.model import Model

"""
I    : origins
J    : destinations
O_i  : capacity of origin i in I
D_j  : request of destination j in J
C_ij : unit transport cost from origin i in I to destination j in J

== Variables
x_ij : amount to be transported from i in I to j in J

== Model
min sum_{i in I, j in J} C_ij x_ij
s.t.
sum_{i in I} x_ij >= D_j        forall j in J
sum_{j in J} x_ij <= O_i        forall i in I
x_ij in R_+/Z_+/{0,1}   (depends on the type of unit of transport)  forall i in I, j in J
"""

# Model creation
model = Model(name="transportation")

# Data (with the refrigerators example)
I = range(3)  # factories A, B and C
J = range(4)  # stores 1, 2, 3 and 4
O = [50, 70, 30]
D = [20, 60, 30, 40]
C = [[6, 8, 3, 4], [4, 2, 1, 3], [4, 2, 6, 5]]

# Variables creation
x = model.integer_var_matrix(I, J, lb=0, name="x")

# Constraints
for j in J:
    model.add_constraint(model.sum(x[i, j] for i in I) >= D[j], ctname=f"request_{j}")
for i in I:
    model.add_constraint(model.sum(x[i, j] for j in J) <= O[i], ctname=f"capacity_{i}")

# Objective function
model.minimize(model.sum(C[i][j] * x[i, j] for i in I for j in J))

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
