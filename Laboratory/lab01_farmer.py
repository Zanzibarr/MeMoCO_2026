from docplex.mp.model import Model

"""
A farmer owns 11 hectares of land where he can grow potatoes or
tomatoes. Beyond the land, the available resources are: 70 kg of tomato
seeds, 18 tons of potato tubers, 145 tons of fertilizer. The farmer knows
that all his production can be sold with a profit of 6000 Euros per hectare
of tomatoes and 7000 Euros per hectare of potatoes. Each hectare of tomatoes
needs 7 kg seeds and 10 tons fertilizer. Each hectare of potatoes needs
3 tons tubers and 20 tons fertilizer. How does the farmer divide his land
in order to gain as much as possible from the available resources?

== Variables
x_t, x_p = # hectares of tomatoes/potatoes

== Model
max 6000 * x_t + 7000 * x_p
s.t.
       1 * x_t +    1 * x_p <=  11 # land
       7 * x_t              <=  70 # tomato seeds
                    3 * x_p <=  18 # potato tubers
      10 * x_t +   20 * x_p <= 145 # fertilizer
x_t, x_p in R_+
"""

# Model creation
m = Model(name="farmer")

# Variables creation
x_t = m.continuous_var(name="x_t", lb=0)
x_p = m.continuous_var(name="x_p", lb=0)

# Constraints
m.add_constraint(x_t + x_p <= 11, ctname="land")
m.add_constraint(7 * x_t <= 70, ctname="tomato_seeds")
m.add_constraint(3 * x_p <= 18, ctname="potato_tubers")
m.add_constraint(10 * x_t + 20 * x_p <= 145, ctname="fertilizer")

# Objective function
m.maximize(6000 * x_t + 7000 * x_p)

# Print solution
if m.solve():
    m.print_solution()
else:
    print(m.get_solve_status())
