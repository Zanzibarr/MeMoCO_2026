from docplex.mp.model import Model

"""
A supermarket chain has a budget W available for opening new stores.
Preliminary analyses identified a set I of possible locations. Opening a
store in i € I has a fixed cost Fi (land acquisition, other administrative
costs etc.) and a variable cost Ci per 100 m2 of store. Once opened, the
store in i guarantees a revenue of Ri per 100 m2. Determine the subset of
location where a store has to be opened and the related size in order to
maximize the total revenue.

I   : candidate locations
W   : available budget
F_i : fixed cost for opening a store in i in I
C_i : variable cost per 100 m2 of store in i in I
R_i : revenue per 100 m2 of store in i in I
M   : big M, upper bound on the size of any store

== Variables
x_i : size of the store in i in I (in 100 m2)
y_i : 1 if a store is opened in i in I, 0 otherwise

== Model
max sum_{i in I} R_i x_i
s.t.
sum_{i in I} (C_i x_i + F_i y_i) <= W
x_i <= M y_i                     forall i in I
x_i in R_+                       forall i in I
y_i in {0,1}                     forall i in I
"""

# Model creation
model = Model(name="supermarket")

# Data
I = range(4)  # candidate locations
W = 1000  # available budget
F = [200, 150, 300, 100]  # F[i] = fixed cost for opening a store in i in I
C = [40, 50, 30, 60]  # C[i] = variable cost per 100 m2 of store in i in I
R = [90, 100, 75, 110]  # R[i] = revenue per 100 m2 of store in i in I
M = W / min(C)  # big M: no store can be larger than the budget allows

# Variables creation
x = model.continuous_var_list(len(I), lb=0, name="x")
y = model.binary_var_list(len(I), name="y")

# Constraints
model.add_constraint(
    model.sum(C[i] * x[i] + F[i] * y[i] for i in I) <= W, ctname="budget"
)
for i in I:
    model.add_constraint(x[i] <= M * y[i], ctname=f"big_M_{i}")

# Objective function
model.maximize(model.sum(R[i] * x[i] for i in I))

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
