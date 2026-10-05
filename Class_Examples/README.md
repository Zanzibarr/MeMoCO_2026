## Run the python scripts (with the community version -- small models only)
1. The first time, create the virtual environment and install the needed packages:
```shell
python3 -m venv .venv && source .venv/bin/activate && pip install cplex docplex
```
2. From within the virtual environment, run the script
```shell
# To enter the (previously created) virtual environment
source .venv/bin/activate
# To run the (farmer) script
python3 l01_farmer.py
```

## Run the python scripts (with a locally installed cplex -- e.g. academic license)
1. Find your CPLEX version:
```shell
# Default Linux path:
ls /opt/ibm/ILOG | grep -i cplex
# Default MacOS path:
ls /Applications | grep -i cplex

# Examples:
# CPLEX_Studio2211 -> version 22.1.1.*
# CPLEX_Studio2212 -> version 22.1.2.*
# ...
```

2. Create a virtual environment and install the C APIs
```shell
python3 -m venv .venv   # or conda/uv
source .venv/bin/activate
pip install "cplex=<version>" docplex

# Example: pip install "cplex=22.1.2.*" docplex
```

3. Link your installation
```shell
docplex config --upgrade /Applications/CPLEX_Studio<version>

# Example: docplex config --upgrade /Applications/CPLEX_Studio2212
```

4. Test your installation
```shell
python3 -c "
from docplex.mp.model import Model
m = Model()
x = m.continuous_var_list(1500, ub=1)
m.maximize(m.sum(x))
print(m.solve().objective_value)
"

# The output should be 1500.0
```

5. Run the script
```shell
# To run the (farmer) script
python3 l01_farmer.py
```

## To run the c++ scripts
1. The first time, or after you make some changes to one of them, compile:
```shell
# To compile one (farmer)
make farmer
# To compile all
make all
```
> !! If you can't compile on your machine, look at the Makefile and change accordingly your compiler, CPLEX path, etc...

2. Run the (farmer) program
```shell
./farmer
```