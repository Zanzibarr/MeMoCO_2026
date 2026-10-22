CC = g++
CPPFLAGS = -g -Wall -O2
LDFLAGS =

# ATTENTION!! UPDATE WITH YOUR ACTUAL PATHS
CPX_BASE    = /Applications/CPLEX_Studio2211
CPX_INCDIR  = $(CPX_BASE)/cplex/include
CPX_LIBDIR  = $(CPX_BASE)/cplex/lib/arm64_osx/static_pic
CPX_LDFLAGS = -lcplex -lm -pthread -ldl

OBJ = transport_timing.o 

%.o: %.cpp
		$(CC) $(CPPFLAGS) -I$(CPX_INCDIR) -c $^ -o $@

main: $(OBJ)
		$(CC) $(CPPFLAGS) $(OBJ) -o transport_timing -L$(CPX_LIBDIR) $(CPX_LDFLAGS)

clean:
		rm -rf $(OBJ) transport_timing

.PHONY: clean
