## To run the python scripts
1. Enter the virtual environment (see the root README.md)
```shell
source .venv/bin/activate
```
2. Run the script
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
./build/farmer
```