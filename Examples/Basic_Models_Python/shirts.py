from docplex.mp.model import Model

"""
A group of kids wants to earn as much as possible by selling decorated
t-shirts and bags to their friends. There are 10 cotton t-shirts and 15
canvas bags available and, for decoration, 32 designed DTF stickers and 40
red trims. Each t-shirt gets 6 stickers and 2 trims, and each bag gets
3 stickers and 5 trims. There are also 15 buttons available, and each bag
uses two of them for its closure. Moreover, 22 labels have been prepared,
to be applied one on each t-shirt and two on each bag. Considering that
each decorated t-shirt is sold for 24 Euros and each bag for 16 Euros, and
that the friends will buy all t-shirts and bags, determine the production
that maximizes the revenue.

== Variables
x_s, x_b = # t-shirts/bags produced

== Model
max 24 * x_s + 16 * x_b
s.t.
         x_s            <= 10 # t-shirts
                    x_b <= 15 # bags
     6 * x_s +  3 * x_b <= 32 # stickers
     2 * x_s +  5 * x_b <= 40 # trims
                2 * x_b <= 15 # buttons
     1 * x_s +  2 * x_b <= 22 # labels
x_s, x_b in Z_+
"""

# Model creation
model = Model(name="shirts")

# Variables creation
x_s = model.integer_var(name="x_s", lb=0)
x_b = model.integer_var(name="x_b", lb=0)

# Constraints
model.add_constraint(x_s <= 10, ctname="tshirts")
model.add_constraint(x_b <= 15, ctname="bags")
model.add_constraint(6 * x_s + 3 * x_b <= 32, ctname="stickers")
model.add_constraint(2 * x_s + 5 * x_b <= 40, ctname="trims")
model.add_constraint(2 * x_b <= 15, ctname="buttons")
model.add_constraint(1 * x_s + 2 * x_b <= 22, ctname="labels")

# Objective function
model.maximize(24 * x_s + 16 * x_b)

# Optimization (log_output=False disables solver output)
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
