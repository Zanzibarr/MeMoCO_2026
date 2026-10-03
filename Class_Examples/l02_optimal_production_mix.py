from docplex.mp.model import Model

"""
I   : resources
J   : products
D_i : availability of resource i in I
P_j : unit profit for product j in J
Q_ij: amount of resource i in I required for each unit of product j in J

== Variables
x_j : amount of product j in J

== Model
max sum_{i in J} P_j x_j
s.t.
sum_{j in J} Q_ij x_j <= D_i     forall i in I
x_j in R_+/Z_+/{0,1} (depends on the type of product)       forall j in J
"""

# Model creation
model = Model(name="production_mix")

# Data (with the perfumes example)
I = range(3)  # rose, lily and violet
J = range(2)  # perfume 1 and perfume 2
D = [27, 21, 9]  # D[i] = availability of resource i in I
P = [130, 100]  # P[j] = profit of resource j in J
Q = [
    [1.5, 1],
    [1, 1],
    [0.3, 0.5],
]  # Q[i][j] = amount of resource i in I required for each unit of product j in J

# Variables creation
x = model.continuous_var_list(len(J), lb=0, name="x")

# Constraints
for i in I:
    model.add_constraint(
        model.sum(Q[i][j] * x[j] for j in J) <= D[i], ctname=f"resource_{i}"
    )

# Objective function
model.maximize(model.sum(P[j] * x[j] for j in J))

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
