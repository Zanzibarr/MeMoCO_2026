from docplex.mp.model import Model
import json

with open("data/lab03_facility_loc_basic_and_plus.json", "r") as file:
    data = json.load(file)

# SETS
I_names = data["I_names"]  # ORDERED set of potential locations
I = range(len(I_names))

# PARAMETERS
model_name = data["modelname"]
W = data["W"]  # available budget
F = data["f_list"]  # ORDERED list of the fixed cost for opening in location i
C = data["c_list"]  # ORDERED list of the variable cost for 100 mq in i
R = data["r_list"]  # ORDERED list of the revenue for 100 mq in i

# parameters for additional constraints (plus)
Max_num = data["max_num_open"]
Min_num = data["min_num_open"]
Min_size = data["min_size_to_open"]
add_constraint_MaxNum = True
add_constraint_MinNum = True

# computed parameters
M = [(W - F[i]) / C[i] for i in I]  # large-enough numbers

# MODEL CREATION
m = Model(name=model_name)

x = m.continuous_var_list(I, lb=0, name="x")
y = m.binary_var_list(I, name="y")

m.maximize(m.sum(R[i] * x[i] for i in I))

m.add_constraint(m.sum(C[i] * x[i] + F[i] * y[i] for i in I) <= W)
m.add_constraints(x[i] <= M[i] * y[i] for i in I)

# additional constraints (plus)
if add_constraint_MaxNum:
    m.add_constraint(m.sum(y[i] for i in I) <= Max_num)
if add_constraint_MinNum:
    m.add_constraint(m.sum(y[i] for i in I) >= Min_num)
m.add_constraints(x[i] >= Min_size * y[i] for i in I)

m.export_as_lp(path=f"{model_name}.lp")

# SOLVE MODEL AND REPORT SOLUTION
if m.solve():
    m.print_solution()
else:
    print(m.get_solve_status())
