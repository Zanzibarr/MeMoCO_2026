# MeMoCO 2026 — Laboratory material

Models, examples and exercises for the MeMoCO course, written in **Python** (DOCPLEX) and **C++** (CPLEX C API).

## Before you start
- Install CPLEX and DOCPLEX: [Instructions/installing_cplex.pdf](Instructions/installing_cplex.pdf)
- Compile C++ programs with CMake (command line, VS Code, Visual Studio): [Instructions/using_cplex_c_api.pdf](Instructions/using_cplex_c_api.pdf)

## Content
```
2026 MeMoCO/
├── cpxmacro.h        # helper macros for the CPLEX C API, shared by every C++ program
├── Instructions/     # how to install CPLEX/DOCPLEX and compile C++ programs
├── Assignments/      # assignments
├── Data/             # JSON datasets read by the Python programs (solutions/: exported solutions)
├── Examples/         # complete models shown in class
├── Exercises/        # exercises: todo/ to complete, solution/ complete
└── Laboratory/       # code written during the laboratory sessions
```

## Running a Python program
Activate the virtual environment, **move into the folder of the program**, and run it:
```bash
source .venv/bin/activate          # Windows: .venv\Scripts\activate
cd Examples/03_json_models
python production_mix.py                         # default dataset
python production_mix.py ../../Data/wines.json   # another dataset
```
Data files are opened with paths relative to the program folder (e.g. `../Data/...`), so running the program from another folder gives `FileNotFoundError`.
In VS Code, enable the setting `python.terminal.executeInFileDir` to make the *Run Python File* button do this automatically.

## Compiling and running a C++ program
Every C++ program has its own folder with a `CMakeLists.txt`: open (or `cd` into) **that** folder.
- **Visual Studio**: *File > Open > Folder...*, select the folder, run with `Ctrl+F5`
- **VS Code**: *File > Open Folder...*, build with `F7`, run with `Shift+F5`
- **Command line**:
  ```bash
  cd Examples/01_basic_models/farmer
  cmake -S . -B build      # only the first time
  cmake --build build      # after every change
  ./build/farmer
  ```

A folder can contain more than one `.cpp` file (e.g. `Exercises/02_moving_scaffolds/solution`). Only the one named in `NAME_DEFAULT` inside `CMakeLists.txt` is compiled: change it, or choose the file from the command line with `cmake -S . -B build -DNAME=moving_scaffolds_map`.

To write a new C++ program, copy the `CMakeLists.txt` of any example into a new folder and set `NAME_DEFAULT`. Then include `cpxmacro.h` with the right relative path, e.g. `#include "../../../cpxmacro.h"`. Do not copy `cpxmacro.h` itself: there is one shared copy at the root of the repository.

Programs write their output files (`.lp` model, `.sol` solution) in the folder they are run from.
