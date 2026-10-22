from docplex.mp.model import Model
from contextlib import contextmanager
import numpy as np
from time import perf_counter

# initialize random number generator
rng = np.random.default_rng(0)


# measuring time
@contextmanager
def timer(label):
    t0 = perf_counter()
    yield
    print(f"=== {label:30}: {perf_counter() - t0:.4f}s ===", flush=True)


# constants
MIN_DIST = 0.5
MAX_DIST = 10
MIN_CAP = 0
MAX_CAP = 100
MIN_REQ = 0
MAX_REQ = 100

# Create random data
n_orig = int(input("Number of origins ? "))
n_dest = int(input("Number of destinations ? "))
thresh = float(input("Distance cutoff ? "))

I = range(n_orig)
J = range(n_dest)
cost = rng.uniform(MIN_DIST, MAX_DIST, size=(n_orig, n_dest))
orig_cap = rng.uniform(MIN_CAP, MAX_CAP, size=n_orig)
dest_req = rng.uniform(MIN_REQ, MAX_REQ, size=n_dest)
orig_cap[0] += max(0, sum(dest_req) - sum(orig_cap))  # make sure it's feasible

with timer("total creation"):
    with timer("pair creation"):
        od_pairs = [(i, j) for i in I for j in J if cost[i, j] <= thresh]

    with timer("variables creation"):
        m = Model(name="transportation with OD pairs")
        x = m.continuous_var_dict(keys=od_pairs, name="x")

    m.minimize(m.sum(cost[od] * x[od] for od in od_pairs))

    with timer("constraint creation"):
        print("Creating destination constraints")
        for j in J:
            if j % 100 == 0:
                print(f"{j} ", end="", flush=True)
            m.add_constraint(
                m.sum(x[od] for od in od_pairs if od[1] == j) >= dest_req[j]
            )
        print("\nCreating origin constraints")
        for i in I:
            if i % 100 == 0:
                print(f"{i} ", end="", flush=True)
            m.add_constraint(
                m.sum(x[od] for od in od_pairs if od[0] == i) <= orig_cap[i]
            )
        print()

with timer("solve"):
    sol = m.solve(log_output=False)

if sol:
    print("objective function: ", m.objective_value)
else:
    print(
        "The model is infeasible: with the generated data, the selected threshold is too small."
    )
