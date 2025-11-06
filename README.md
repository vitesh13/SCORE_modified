## Getting started
SCORE targets computers running a Unix operating system. Windows11/WSL2 is also possible.

0. Clone this repository.
```
git clone https://www.github.com/vitesh13/SCORE_modified.git
cd SCORE_modified
git checkout prepare_v1.3.0
echo $PWD
```

`SCORE_modified` is the home directory of the installation.

1. The command on the last line returns an absolute path to the current working
   directory. In what follows, this is considered the `MYPROJECTPATH`, or home
   directory. This directory includes this README.md and the `CMakeLists.txt`
   build recipe to compile `score` from its source code. But before doing one
   has to check if all relevant dependencies and libraries are installed on
   the computer which will be required to compile `score`.
   These are: `make`, `build-essentials`, `make`, `mpich`, `libssl-dev`, and `wget`.
   If some are missing these can be installed via apt.

2. Compile the HDF5 library from its source code `src/thirdparty/hdf5`
```
# following the instructions in detailed in
src/cxx/thirdparty/hdf5/README.md
```

3. Compile SCORE by navigating into the build directory, configure, and compile:
```
mkdir -p build
cd build
cmake -DGITSHA=`git rev-parse HEAD` -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_COMPILER=mpicxx ..
make -j8
```

4. Provided the previous step was successful, make `score` executable:
```
chmod +x score
```

## Run the most basic example:
A single crystal growing into a homogeneously deformed continuum.
```
cd examples/verification
export OMP_NUM_THREADS=1
mpirun -n 1 ./../../build/score 0 SCORE.Verification.uds
```

## Run the specific DAMASK example:
```
cd examples/damask
export OMP_NUM_THREADS=1
mpirun -n 1 ./../../build/score 1 SCORE.Debug.MartinDiehl.64.SimID.6403_damask.uds $PWD/Tphase_Vf_01_tensionX_remeshed_increment19000_segmented.dream3D
```

## Analysis and display via Python:
Inspect and post-process the data using a Python virtual environment and
a jupyter notebook for example. We show the necessary step to create a virtual
environment and how to install jupyterlab and jupyterlab_h5web into this
environment. We assume a version of Python is available, either via
conda/miniconda. In what follows exemplified for Python3.13.
Execute within the project home directory. 

```
python3.13 -m venv .py3.13
source .py3.13/bin/activate
python -m pip install --upgrade pip
python -m pip install jupyterlab
python -m pip install jupyterlab_h5web
jupyter-lab
# inspect the example script in examples/Inspect.SCORE.Results.JupyterH5Web.ipynb
```

## Obsolete optional stuff:

Install rapidyaml
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

