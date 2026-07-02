# Installation and compilation guide

Third-party dependencies

```
`build-essentials`
`cmake`
`mpich`
`libssl-dev`
`wget`
`git`
```

Use for checking if these libraries are present

```
./SCORE.Prerequisites.sh
```

### Compile the `HDF5` library from its source code `src/cxx/thirdparty/hdf5`

A few C/C++ libraries are required for `SCORE` to work correctly. While most of these
we pull in automatically via the `git submodule` step, the `HDF5` library is a trickier
one to take care of. If this library is not present on the host already instructions
we have composed instructions to compile the library locally with all steps already
pre-configured and scripts for automation available here

```
# follow the instructions as detailed in
src/cxx/thirdparty/hdf5/README.md
```

<!--
## What should you know before this tutorial?

To get started, it does not hurt to read the following `pynxtools` tutorial:

- [Guide on getting started with `pynxtools`, NeXus, and NOMAD](https://fairmat-nfdi.github.io/pynxtools/getting-started.html)
- [Installation tutorial for `pynxtools`](https://fairmat-nfdi.github.io/pynxtools/tutorial/installation.html)

## What you will know at the end of this tutorial?

You will know

- how to install `pynxtools-score`
- how to install `pynxtools-score` together with NOMAD

## Setup

It is recommended to use python 3.12 with a dedicated virtual environment for this package. Learn how to manage [python versions](https://github.com/pyenv/pyenv) and [virtual environments](https://realpython.com/python-virtual-environments-a-primer/).

There are many alternatives to managing virtual environments and package dependencies (requirements). We recommend using [`uv`](https://github.com/astral-sh/uv), an extremely fast Python package and project manager. In this tutorial, you will find parallel descriptions, using either `uv` or a more classical approach using `venv` and `pip`.

Start by creating a virtual environment:

=== "uv"
    `uv` is capable of creating a virtual environment and install the required Python version at the same time.

    ```bash
    uv venv --python 3.12
    ```

=== "venv"

    Note that you will need to install the Python version manually beforehand.

    ```bash
    python -m venv .venv
    ```
That command creates a new virtual environment in a directory called `.venv`.

## Installation

Install the latest stable version of this package from PyPI with

=== "uv"

    ```bash
    uv pip install pynxtools-score
    ```

=== "pip"

    ```bash
    pip install pynxtools-score
    ```

You can also install the latest _development_ version with

=== "uv"

    ```bash
    uv pip install git+https://github.com/FAIRmat-NFDI/pynxtools-score.git
    ```

=== "pip"

    ```bash
    pip install git+https://github.com/FAIRmat-NFDI/pynxtools-score.git
    ```

### How to install `pynxtools-score` with NOMAD

To use `pynxtools-score` with NOMAD, simply install it in the same environment as the `nomad-lab` package. NOMAD will recognize `pynxtools-score` as a plugin automatically. In addition, NOMAD will install a schema for NeXus application definitions.

## Start using `pynxtools-score`

That's it! You can now use `pynxtools-score`!
-->


<!--4. Install `Python` tools to generate configuration files and enable access to visualization tools

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
```-->


<--## Obsolete stuff:

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
so that the paths point to an already installed version.-->


<!--The first example is a useful test case for exploring how a single nucleus grows into a
deformed matrix. For developers the simulation is here kept minimal in size and is
also useful as it allows to explore the numerical implementation details like transfer
of matter across boundaries between parallel threads, all details happening in the
recrystallization front for each cell.

The second example is a useful test case for exploring the coupled DAMASK/SCORE simulation,
to learn how the data transfer takes care. Note that the output file from DREAM.3D
is referred to in the configuration file including a SHA256 hashes to assure input and
output are always well defined and reproducible.-->

