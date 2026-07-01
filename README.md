## Getting started

`SCORE` targets computers running a Unix operating system. Windows11/WSL2 is also possible.

0. Assure you have a working C++ compiler, e.g., `GNU` or `Intel` installed, alongside

```
`build-essentials`
`cmake`
`mpich`
`libssl-dev`
`wget`
`git`
```

If some of these dependencies are missing they can be installed, at the host system level, e.g., through `apt`.

Once installed or to check their presence use the following commands

```
gcc --version
g++ --version
# above two lines exemplify for GNU compiler, may need a change, e.g., icx and icpx respectively when using the Intel compiler
cmake --version
mpiexec --version
openssl version -a
wget --version
git --version
```

1. Clone this repository

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

2. Compile the `HDF5` library from its source code `src/cxx/thirdparty/hdf5`

A few C/C++ libraries are required for `SCORE` to work correctly. While most of their source code
is set by the `git submodule update` step, it remains to compile a version of the `HDF5` library.
If this library is not present on the host already instructions on how to go about doing this
are given here

```
# follow the instructions as detailed in
src/cxx/thirdparty/hdf5/README.md
```

3. Compile `SCORE` by navigating into the build directory, configure, and compile

A script is available to support you. Modify the script as needed to define
how many computing resources to use or if you plan to install in places
other than here exemplified project root directory.

```
./SCORE.Build.sh
```

Once compiled, the C++ executable `score` is executable from its location in `build`.

4. Install `Python` tools to generate configuration files and enable access to visualization tools

Configuration files for `SCORE` and analyses of results from `SCORE` simulations are created using `Python`.
For this working with a virtual environment is recommended. Several routes exists for setting up a virtual environment.
One is [uv](https://docs.astral.sh/uv/), or classical python [venv](https://docs.python.org/3/library/venv.html), or
[conda](https://www.anaconda.com/docs/getting-started/miniconda/main).

Exemplified for a Unix system (tested on Ubuntu), we show the route using `uv` and the relevant command line instructions.

If `uv` is not installed yet on your system you can proceed as follows

```
# assure that you have no virtual environment active i.e.
# no entry like (name) left to typical command line username@computername:
curl -LsSf https://astral.sh/uv/install.sh | sh
```

Once `uv` is installed (version `0.4.0` or newer recommended), create a virtual environment with
the desired Python version, activate it, and ensure pip is up to date:

```
uv venv --python 3.12 score_venv_py3.12
source score_venv_py3.12/bin/activate

uv pip install --upgrade pip
uv pip list
which python
which pip
```

The `uv venv --python 3.12` command will automatically download and install `Python 3.12`
if it is not already available on your system.

We can now install Python packages that can be useful when working with data from SCORE

```
uv pip install jupyterlab
uv pip install jupyterlab_h5web
uv pip install flatdict
# and maybe add pynxtools to have a tool for verifying how NeXus/HDF5 files generated
# for SCORE comply with the NXmicrostructure_score application definitions
# note that for just performing simulations with SCORE the next line is not
uv pip install pynxtools
```

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

## How to run your own simulations

With these suggestions and tutorials you have a blue-print how to create your own `SCORE` and `DAMASK`
configurations. With the configuration modernized and expressed now in `Python` and having the
underlying data models formalized using the [`NeXus`](https://manual.nexusformat.org/classes/contributed_definitions/micro-structure.html) standard,
you will be well equipped to run your own parametric studies and work with `SCORE`.

At the latest at this point it is important to consider that the examples and the parameterization is meant as examples.
It is your responsibility to check if these match physically with the expectations and realities of your own research.
 
## Numerical details

SCORE uses a deterministic algorithm. A pseudo-random number generator is used that is seeded deterministically.


<!--The first example is a useful test case for exploring how a single nucleus grows into a
deformed matrix. For developers the simulation is here kept minimal in size and is
also useful as it allows to explore the numerical implementation details like transfer
of matter across boundaries between parallel threads, all details happening in the
recrystallization front for each cell.

The second example is a useful test case for exploring the coupled DAMASK/SCORE simulation,
to learn how the data transfer takes care. Note that the output file from DREAM.3D
is referred to in the configuration file including a SHA256 hashes to assure input and
output are always well defined and reproducible.

## Work with JupyterNotebook

```
# make sure to have OMP_NUM_THREADS set before starting jupyter-lab, i.e.
export OMP_NUM_THREADS=1
jupyter-lab
```

## Numerical details

SCORE uses a deterministic algorithm. A pseudo-random number generator is used that is seeded deterministically.

We can also take advantage of functionalities provided by the pynxtools-em, which
is a plugin of pynxtools to get functionalities that allow us to generate
a human-readable summary of the binary content in the HDF5 files as a flat
list of HDF5 path and values.

## Obsolete stuff:

Optional installation of rapidyaml
```
# python e.g. 3.11
# git clone --recursive https://github.com/biojppm/rapidyaml.git
# cd tools && python amalgamate.py ryml_all.hpp && cd ..
# mkdir build && cd build
# cmake -DCMAKE_BUILD_TYPE=Release ..
# make -j8 && cd ..
```

   Alternatively customize the reference to this library in `CMakeLists.txt`
   so that the paths point to an already installed version.
-->
