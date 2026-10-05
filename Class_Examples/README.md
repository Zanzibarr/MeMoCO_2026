## To run the python scripts
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