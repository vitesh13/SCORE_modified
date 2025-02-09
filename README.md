## Getting started
SCORE targets computers running a Unix operating system. Windows11/WSL2 is also possible.

0. Clone this repository.

```
git clone git@github.com:vitesh13/SCORE_modified.git
cd SCORE_modified
git checkout nfdi-nexus-amag
echo $PWD
```

1. The command on the last line returns an absolute path to the current working
   directory. In what follows, this is considered the `MYPROJECTPATH`, or home
   directory. This directory includes this README.md and the `CMakeLists.txt`
   build recipe to compile `score` from its source code. But before doing one
   has to check if all relevant dependencies and libraries are installed on
   the computer which will be required to compile `score`.
   These are: `make`, `build-essentials`, `make`, `mpich`, `libssl-dev`, and `wget`.
   If some are missing these can be installed via apt.

2. Compile the HDF5 library from its source code `src/thirdparty/hdf5`.


Install zlib in an install e.g. https://github.com/madler/zlib
```
./configure --prefix=$PWD/install
make
make install
```
Install szlib in an install e.g. instructions https://github.com/erdc/szip/blob/master/INSTALL
```
./configure --prefix=$PWD/install
make
make install
```

Install hdf5, configure config/cmake/scripts/HDF5Options.cmake, v1.14.6, see release_docs/INSTALL_CMake.txt
settings end up in lib/libhdf5.settings
```
mkdir -p src/cxx/thirdparty/mandatory/hdf5
cd src/cxx/thirdparty/mandatory/hdf5
wget https://support.hdfgroup.org/ftp/HDF5/releases/hdf5-1.14/hdf5-1.14.0/src/CMake-hdf5-1.14.0.tar.gz
tar -xvf CMake-hdf5-1.14.0.tar.gz && cd CMake-hdf5-1.14.0
./build-unix.sh 2>&1 | tee PARAPROBE.Hdf5.Build.STDOUTERR.txt
./HDF5-1.14.0-Linux.sh --include-subdir --skip-license 2>&1 | tee PARAPROBE.Hdf5.Install.STDOUTERR.txt
cd ../../../../../../
```

   Alternatively customize the reference to this library in `CMakeLists.txt`
   so that the paths point to an already installed version.

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

5. Run the most basic example:
Single crystal expansion into homogeneously deformed continuum.


```
cd examples/verification
export OMP_NUM_THREADS=1
mpirun -n 1 ./../../build/score 0 SCORE.Verification.uds
```

6. Run the specific DAMASK example:

```
cd examples/damask
export OMP_NUM_THREADS=1
mpirun -n 1 ./../../build/score 1 SCORE.Debug.MartinDiehl.64.SimID.6403_damask.uds $PWD/Tphase_Vf_01_tensionX_remeshed_increment19000_segmented.dream3D
```

7. Inspect and post-process the data using a Python virtual environment and
   a jupyter notebook for example. We show the necessary step to create a virtual
   environment and how to install jupyterlab and jupyterlab_h5web into this
   environment. We assume a version of Python is available, either via
   conda/miniconda or via apt such as the excellent deadsnakes Python packages
   for apt. Here exemplified for Python3.8. Execute within the project home
   directory.

```
python3.8 -m venv .py38
source .py38/bin/activate
python -m pip install --upgrade pip
python -m pip install jupyterlab
python -m pip install jupyterlab_h5web
jupyter-lab
# inspect the example script in examples/Inspect.SCORE.Results.JupyterH5Web.ipynb
```
