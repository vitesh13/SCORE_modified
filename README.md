## Getting started

SCORE targets computers running a Unix operating system. Windows11/WSL2 is also possible.

0. Assure you have a working C++ compiler e.g., GNU or Intel installed, alongside
`build-essentials`, `cmake`, `mpich`, `libssl-dev`, `wget`, and `git`. If some are missing these can be installed e.g., through apt.
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



1. Clone this repository.
```
git clone https://www.github.com/vitesh13/SCORE_modified.git
cd SCORE_modified
git checkout prepare_v1.3.0
git submodule update --recursive
# git submodule update --init --recursive
echo $PWD
```

The last command identifies `SCORE_modified`, the home directory of the installation.
This directory is mentioned in the variable `MYPROJECTPATH` in the `CMakeLists.txt`
file to assure the project directory is correctly configured for building, i.e.,
compiling and linking of the C++ executable `score`.

2. Compile the HDF5 library from its source code `src/thirdparty/hdf5`
A few C/C++ libraries are required for SCORE to work correctly. While most
of their source code is set by the git submodule update step, HDF5 is a library
that needs to compile from scratch in the present configuration. For this
```
# follow the instructions as detailed in
src/cxx/thirdparty/hdf5/README.md
```

3. Compile SCORE by navigating into the build directory, configure, and compile:
```
./SCORE.Build.sh
```

Once compiled, the C++ executable `score` is executable from its location in `build`.

4. Install Python tools to generate configuration files and enable access to visualization tools
Configuration files for SCORE and analyses of results from SCORE simulations are created using Python.
For this working with a virtual environment is recommended. Several routes exists for setting up
a virtual environment. We recommend [uv](https://docs.astral.sh/uv/). Others are [conda](https://www.anaconda.com/docs/getting-started/miniconda/main) or classical python [venv](https://docs.python.org/3/library/venv.html).
Exemplified for a Unix system (tested on Ubuntu), we show the route using uv and the relevant command line instructions.

If this is not installed yet on your system e.g.
```
# assure that you have no virtual environment active i.e.
# no entry like (name) left to typical command line username@computername:
curl -LsSf https://astral.sh/uv/install.sh | sh
```

Once uv is installed, we can create an exemplar environment with a specific
python version via uv, activate it, and install the latest pip version:
```
uv venv score_venv_py3.13 --python 3.13
source score_venv_py3.13/bin/activate
uv pip install --upgrade pip
# next line optional to display what has been installed in that environment
uv pip list
# next line optional to display where the environment is stored
which uv
# next line optional to display where the python interpreted is in that environment
which python
```

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

## Run the most basic example:

A single crystal growing into a homogeneously deformed continuum.
```
cd examples/verification
export OMP_NUM_THREADS=1
mpirun -n 1 ./../../build/score 0 SCORE.Config.SimID.0.nxs
```

## Run a more specific example:

A deformed microstructure from a DAMASK simulation coupled to SCORE.
The DAMASK simulation was post-processed with DREAM.3D. Specifically,
which specific locations in the RVE were flagged with a marker using
a nucleation model that was developed by the AMAG to instruct SCORE
to place nuclei in these flagged locations. SCORE then simulates the
growth of these nuclei.
```
cd examples/damask
export OMP_NUM_THREADS=1
mpirun -n 1 ./../../build/score 1 SCORE.Config.SimID.1.nxs
```

## How to run your own simulations:

There is a simple JupyterNotebook with which you can create own SCORE.Config configuration
files. The details how to instantiate or change the parameterization of the settings for the
two above-mentioned examples can serve as an example how to proceed with your own studies
using SCORE. This Python code has been kept minimal to assist users that are primarily
interested in applying SCORE. Parameters such as the deformed microstructure, material,
nucleation model, grain boundary mobility can be edited.
The code in the JupyterNotebook as intentionally been kept simple to enable you to
interface it with custom scripts for running parametric studies.

```
jupyter-lab
# inspect the example script in rc/python/NXmicrostructure_score_config.ipynb
```

Note that the exemplar values here provided are really examples only!

The first example is a useful test case for exploring how a single nucleus grows into a
deformed matrix. For developers the simulation is here kept minimal in size and is
also useful as it allows to explore the numerical implementation details like transfer
of matter across boundaries between parallel threads, all details happening in the
recrystallization front for each cell.

The second example is a useful test case for exploring the coupled DAMASK/SCORE simulation,
to learn how the data transfer takes care. Note that the output file from DREAM.3D
is referred to in the configuration file including a SHA256 hashes to assure input and
output are always well defined and reproducible.

## Work with JupyterNotebook

We can then start jupyterlab to display the NeXus/HDF5 files that were generated
by SCORE.
```
jupyter-lab
# inspect the example script in examples/Inspect.SCORE.Results.JupyterH5Web.ipynb
```

## Numerical details

SCORE uses a deterministic algorithm.
A pseudo-random number generator is used
that is seeded deterministically.

<!--
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
