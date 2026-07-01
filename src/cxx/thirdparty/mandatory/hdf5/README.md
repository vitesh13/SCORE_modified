## `HDF5`, the Hierarchical Data Format Version 5, is a library for scientific computing I/O

All input and output to `SCORE` uses the `HDF5` library.

### Version used
This directory serves as a place to use a local installation of the `HDF5` library.
The directory is pre-configured for compiling `HDF5-1.14.6` from its source code
`hdf5-1.14.6.tar.gz` within the `CMake` package. The `INSTALL_CMake.txt` document
in the release_docs directory that comes with this HDF5 versions states that several
other `HDF5 plugins` **need to be placed in the directory** where this `README.md` is located
such that the libraries can be compiled successfully.

In summary, one should use the following recipe to compile the library:

### Download the HDF5 source code archive

```
wget https://github.com/HDFGroup/hdf5/archive/refs/tags/hdf5_1.14.6.tar.gz
```

### Download the relevant source code archives for key plugins

```
wget https://github.com/HDFGroup/hdf5_plugins/releases/download/hdf5-1.14.6/hdf5_plugins-1.14.tar.gz
wget https://github.com/madler/zlib/releases/download/v1.3.1/zlib-1.3.1.tar.gz
wget https://github.com/zlib-ng/zlib-ng/archive/refs/tags/2.2.5.tar.gz
wget https://github.com/MathisRosenhauer/libaec/releases/download/v1.1.3/libaec-1.1.3.tar.gz
```

Compressed archives are not stored in the SCORE Github repository to reduces its size.
Make sure all these tar files are in the folder where this `README.md` is located. 

### Create a build directory in the folder of README.md and unpack the HDF5 archive but do not unpack the plugins

```
mkdir -p build
tar -xvf hdf5-1.14.6.tar.gz
```

It could be that the tar file of the HDF5 gets downloaded with a different name. 

Make sure that the tar file is named in this format `hdf5-1.14.6.tar.gz`.

### Compile and test as per the release_docs suggestion

```
ctest -S HDF5config.cmake,BUILD_GENERATOR=Unix -C Release -VV -O hdf5.log
```

### Install the library locally with skip-license a convenience call

```
./HDF5-1.14.6-Linux.sh --include-subdir --skip-license 2>&1
```

Configuration files `CTestScript.cmake`, `HDF5config.cmake`, and `HDF5options.cmake`
are preconfigured for typical usage of SCORE running as a single MPI process.

