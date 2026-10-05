# Installing CPLEX and DOCPLEX

## CPLEX installation
1. Install CPELX through the SkillsBuild initiative: https://www.ibm.com/academic/

2. Find the installation path
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

3. Create a virtual environment and install the C APIs
```shell
# Create and enter the virtual environment
python3 -m venv .venv   # or with conda or uv, your choice
source .venv/bin/activate

# Install the DOCPLEX/CPLEX APIs
pip install "cplex=<version>" docplex
# Example: pip install "cplex=22.1.2.*" docplex
```

4. Link your installation
```shell
docplex config --upgrade /Applications/CPLEX_Studio<version>

# Example: docplex config --upgrade /Applications/CPLEX_Studio2212
```

5. Test your installation
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
