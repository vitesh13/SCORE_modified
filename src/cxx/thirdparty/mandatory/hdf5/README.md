## Hierarchical Data Format (HDF5) scientific computing I/O library

All input and output to `SCORE` uses the `HDF5` library. Use the following recipe to compile the library

### Preparation script

Compressed archives are not stored in the `SCORE` Github repository to reduces its size.
Download the required artifacts, create a build directory in the folder of `README.md`,
and unpack the `HDF5` archive but do not unpack the plugins.

```
./DownloadHdfFiveAndPlugins.sh
```

### Compile and test as per the release_docs suggestion

```
ctest -S HDF5config.cmake,BUILD_GENERATOR=Unix -C Release -VV -O hdf5.log
```

### Install the library locally with skip-license a convenience call

```
./HDF5-1.14.6-Linux.sh --include-subdir --skip-license 2>&1
```

Configuration files `CTestScript.cmake`, `HDF5config.cmake`, and `HDF5options.cmake`
are pre-configured for typical usage of `SCORE` running as a single MPI process.


### Version used

This directory serves as a place to use a local installation of the `HDF5` library.
The directory is pre-configured for compiling `HDF5-1.14.6` from its source code
`hdf5-1.14.6.tar.gz` within the `CMake` package. The `INSTALL_CMake.txt` document
in the release_docs directory that comes with this HDF5 versions states that several
other `HDF5 plugins` **need to be placed in the directory** where this `README.md` is
located such that the libraries can be compiled successfully.

