## Getting started

`SCORE` targets computers running a Unix operating system. Windows11/WSL2 is also possible.

### 1. Clone this repository

```
git clone https://www.github.com/vitesh13/SCORE_modified.git
cd SCORE_modified
git checkout master
git submodule sync ---recursive
git submodule update --init --recursive --jobs==4
echo $PWD
```

The last command identifies the home directory of the installation.
This directory is mentioned in the variable `MYPROJECTPATH` in the `CMakeLists.txt`
file to assure the project directory is correctly configured for building, i.e.,
compiling and linking of the C++ executable `score`.

### 2. Provide third-party dependencies via setting up a conda environment

Assure you have a working and the  installed
A [`conda` environment](https://www.anaconda.com/docs/getting-started/miniconda/main) and recipe is provided.
This environment includes all relevant third-party dependencies, like a [C/C++ compiler (e.g. `GNU`)](https://gcc.gnu.org/),
the [Message Passing Interface library (`MPI`)](https://www.mpich.org/), `Python`, `JupyterLab`, `DAMASK`, and other relevant dependencies.
The environment can be installed like this

```
conda env create -f conda.yaml
```

Thereafter, install the `score-recrystallization` Python module which is used for generating configuration files for SCORE.

```
pip install -e "."
```

Alternatively, `SCORE` can be installed on your host system, i.e. locally, which requires dependencies installed via a package manager
for the operating system of choice, e.g., `apt`. See the installation instructions in `docs/tutorial/installation.md`.

### 3. Compile `SCORE` by navigating into the build directory, configure, and compile

A script is available to support you. Modify the script as needed to define
how many computing resources to use or if you plan to install in places
other than here exemplified project root directory.

```
./SCORE.Build.sh
```

Once compiled, the C++ executable `score` is executable from its location in `build`.

## Run the most basic example

A single crystal growing into a homogeneously deformed continuum.

```
cd examples/verification
export OMP_NUM_THREADS=1
mpirun -n 1 ./../../build/score 0 SCORE.Config.SimID.0.nxs
```

## Run a more specific example

A deformed microstructure from a [`DAMASK`](https://damask-multiphysics.org/) simulation coupled to `SCORE`.

```
cd examples/damask
export OMP_NUM_THREADS=1
mpirun -n 1 ./../../build/score 1 SCORE.Config.SimID.1.nxs
```

## Run the example from the research paper

## Run your own simulations

With these suggestions and tutorials you have a blue-print how to create your own `SCORE` and `DAMASK`
configurations. With the configuration modernized and expressed now in `Python` and having the
underlying data models formalized using the [`NeXus`](https://manual.nexusformat.org/classes/contributed_definitions/micro-structure.html) standard,
you will be well equipped to run your own parametric studies and work with `SCORE`.

At the latest at this point it is important to consider that the examples and the parameterization is meant as examples.
It is your responsibility to check if these match physically with the expectations and realities of your own research.
 
## Numerical details

SCORE uses a deterministic algorithm. A pseudo-random number generator is used that is seeded deterministically.

## Work with JupyterNotebook

```
# make sure to have OMP_NUM_THREADS set before starting jupyter-lab, i.e.
export OMP_NUM_THREADS=1
jupyter-lab
```

<!--We can also take advantage of functionalities provided by the pynxtools-em, which
is a plugin of pynxtools to get functionalities that allow us to generate
a human-readable summary of the binary content in the HDF5 files as a flat
list of HDF5 path and values.-->


