# SPDX-License-Identifier: GPL-3.0-or-later

"""Default parameterization of the SCORE model."""


import os
from pathlib import Path
import numpy as np

import h5py


def test_score_example_verification_ran_successfully():
    """Check that the HDF5 file exists."""
    file_path = f"{os.getcwd()}/examples/verification/SCORE.Results.SimID.0.nxs"
    assert Path(file_path).exists()
    with h5py.File(file_path) as h5r:
        assert np.isclose(
            h5r["/entry1/spatiotemporal1/microstructure100/x"][...],
            0.9983775979864946,
            atol=1e-6
        )
