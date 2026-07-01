## Install legacy DREAM3D for synthesizing microstructures

This is a location where to store a version of legacy `DREAM3D` to synthesize microstructures.
Legacy `DREAM3D` is currently required for running `examples/amag/workflow.ipynb`.

We strongly suggest to install legacy `DREAM3D` [`v6.5.171`](https://github.com/BlueQuartzSoftware/DREAM3D/releases)
into this directory, so that after the installation `thirdparty/dream3d/DREAM3D-6.5.171-Linux-x86_64` exists
within home directory of the `SCORE` installation. The home directory is there one which holds also the `docs`, `src`,
`thirdparty` sub-directories and `pyproject.toml`, `CITATION.cff` files.

The `examples/amag/workflow.ipynb` is configured such that the installation writes the synthesized microstructure
to `examples/amag` into a file named `Microstructure_for_DAMASK.dream3d`. The notebook is preconfigured to search
for the installation of the legacy `DREAM3D` in `thirdparty/dream3d/DREAM3D-6.5.171-Linux-x86_64`. Correspondingly,
the pipeline instruction file `Microstructure_for_DAMASK.json` is configured to write into `examples/amag`.



