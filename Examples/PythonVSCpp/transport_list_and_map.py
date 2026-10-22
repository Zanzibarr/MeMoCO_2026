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
# 1: all vars at once, then map
# 2: var and mapping one at a time (sensibly slower)
MODE = 1

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
        # x[p] is the variable x_ij, where (i, j) == od_pairs[p]
        # map_x[i, j] is the position of x_ij in x, -1 if x_ij does not exist
        map_x = np.full((n_orig, n_dest), -1, dtype=int)
        if MODE == 1:
            x = m.continuous_var_list(keys=len(od_pairs), name="x")
            for p, (i, j) in enumerate(od_pairs):
                map_x[i, j] = p
        else:
            x = []
            for p, (i, j) in enumerate(od_pairs):
                x.append(m.continuous_var(name=f"x_{i}_{j}"))
                map_x[i, j] = p

    m.minimize(m.sum(cost[od] * x[p] for p, od in enumerate(od_pairs)))

    with timer("constraint creation"):
        c = []
        print("Creating destination constraints")
        for j in J:
            if j % 100 == 0:
                print(f"{j} ", end="", flush=True)
            c.append(
                m.sum(x[map_x[i, j]] for i in I if map_x[i, j] >= 0) >= dest_req[j]
            )
        print("\nCreating origin constraints")
        for i in I:
            if i % 100 == 0:
                print(f"{i} ", end="", flush=True)
            c.append(
                m.sum(x[map_x[i, j]] for j in J if map_x[i, j] >= 0) <= orig_cap[i]
            )
        print()
        m.add_constraints(c)

with timer("solve"):
    sol = m.solve(log_output=False)

if sol:
    print("objective function: ", m.objective_value)
else:
    print(
        "The model is infeasible: with the generated data, the selected threshold is too small."
    )
