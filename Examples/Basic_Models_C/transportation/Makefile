CC = g++
CPPFLAGS = -g -Wall -O2 -std=c++11

NAME = transportation

# ATTENTION!! UPDATE WITH YOUR ACTUAL PATHS
CPX_BASE    = /Applications/CPLEX_Studio2211
CPX_INCDIR  = $(CPX_BASE)/cplex/include
ARCH = arm64_osx
CPX_LIBDIR  = $(CPX_BASE)/cplex/lib/$(ARCH)/static_pic
CPX_LDFLAGS = -lcplex -lm -pthread -ldl

all: $(NAME)

%.o: %.cpp
		$(CC) $(CPPFLAGS) -I$(CPX_INCDIR) -c $^ -o $@

$(NAME): $(NAME).o
		$(CC) $(CPPFLAGS) $(NAME).o -o $(NAME) -L$(CPX_LIBDIR) $(CPX_LDFLAGS)

clean:
		rm -rf $(NAME).o $(NAME)

.PHONY: all clean