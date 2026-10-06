# Usage: make <name>   e.g. make farmer, make shirts, make transportation
#        make all      builds every lNN_<name>.cpp
#        make clean

CPLEX_DIR  ?= /Applications/CPLEX_Studio2212
ARCH       ?= arm64_osx
LIBFORMAT  := static_pic

CXX      := clang++
CXXFLAGS := -std=c++23 -O2 -Wall -DIL_STD -DNDEBUG -fPIC -fexceptions -fno-strict-aliasing \
            -I$(CPLEX_DIR)/cplex/include -I$(CPLEX_DIR)/cplex/include/ilcplex -I$(CPLEX_DIR)/concert/include -I..
LDFLAGS  := -L$(CPLEX_DIR)/cplex/lib/$(ARCH)/$(LIBFORMAT) -L$(CPLEX_DIR)/concert/lib/$(ARCH)/$(LIBFORMAT)
LDLIBS   := -lilocplex -lconcert -lcplex -lm -lpthread

# l01_farmer.cpp -> farmer, l02_minimum_cost_covering.cpp -> minimum_cost_covering, ...
SRCS  := $(wildcard l[0-9][0-9]_*.cpp)
NAMES := $(shell echo $(SRCS:.cpp=) | tr ' ' '\n' | sed -E 's/^l[0-9]+_//')

BUILD_DIR := build
BINS      := $(addprefix $(BUILD_DIR)/,$(NAMES))

.PHONY: all clean list $(NAMES)
all: $(BINS)

# make farmer -> builds build/farmer
$(NAMES): %: $(BUILD_DIR)/%

.SECONDEXPANSION:
$(BINS): $(BUILD_DIR)/%: $$(wildcard l[0-9][0-9]_%.cpp) ../cplex_utils.hpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $< -o $@ $(LDFLAGS) $(LDLIBS)

$(BUILD_DIR):
	mkdir -p $@

list:
	@echo $(NAMES)

clean:
	rm -f $(BINS)
