# SPDX-License-Identifier: GPL-3.0-or-later

import os

import numpy as np
from pint import Quantity
from score_config_utils import ureg

MY_MATERIAL: list[tuple[str, Quantity]] = [
    (
        "melting_temperature",
        ureg.Quantity(np.float64(650.0), ureg.degC).to(ureg.kelvin),
    ),
    (
        "shear_modulus_zero",
        ureg.Quantity(np.float64(26.0), ureg.gigapascal).to(ureg.pascal),
    ),
    ("nadal_lepoac_a", ureg.Quantity(np.float64(0.469))),
    ("nadal_lepoac_zeta", ureg.Quantity(np.float64(0.04))),
    ("burgers_vector_zero", ureg.Quantity(np.float64(0.286), ureg.nanometer)),
    # shown here how to consider a constant Burgers vector
    ("lattice_expansion_second", ureg.Quantity(0.0, (ureg.kelvin**-2))),
    ("lattice_expansion_first", ureg.Quantity(0.0, (ureg.kelvin**-1))),
    ("lattice_expansion_null", ureg.Quantity(0.0, ureg.nanometer)),
]

MY_USER: list[str] = [
    "Vitesh Shah",
    "Markus Kühbach",
    "Sebastian Samberger",
    "Georg Falkinger",
    "Franz Roters",
]

MY_INIT_DAMASK: dict[str, str] = {
    "file_path": f"{os.getcwd()}/../../examples/damask/Tphase_Vf_01_tensionX_remeshed_increment_19000_segmented.dream3d"
}

MY_MOBILITY_RH: list[tuple[str, Quantity]] = [
    (
        "m_null",
        ureg.Quantity(np.float64(300.0), ureg.meter**4 / (ureg.joule * ureg.second)),
    ),
    ("enthalpy", ureg.Quantity(np.float64(2.0314), ureg.eV)),
    # ("c1", ureg.Quantity(np.float64(0.9))),
    # ("c2", ureg.Quantity(np.float64(5.))),
    # ("c3", ureg.Quantity(np.float64(9.))),
]

MY_TIME_TEMPERATURE: list[tuple[str, Quantity]] = [
    (
        "time",
        ureg.Quantity(
            np.asarray(
                [
                    0.0,
                    5.8,
                    100.8,
                    103.7,
                    105.0,
                    200.8,
                    205.8,
                    208.7,
                    300.0,
                    305.8,
                    405.8,
                    408.7,
                    600.0,
                ],
                np.float64,
            ),
            ureg.second,
        ),
    ),
    (
        "temperature",
        ureg.Quantity(
            np.asarray(
                [
                    20.0,
                    500.0,
                    500.0,
                    20.0,
                    20.0,
                    500.0,
                    500.0,
                    20.0,
                    20.0,
                    500.0,
                    500.0,
                    20.0,
                    20.0,
                ],
                np.float64,
            ),
            ureg.degC,
        ).to(ureg.kelvin),
    ),
]

MY_GRID_CUBOIDAL: list[tuple[str, Quantity]] = [
    ("extent", ureg.Quantity(np.asarray([19, 15, 11], np.uint32))),
    ("cell_dimensions", ureg.Quantity(np.float64(0.5), ureg.micrometer)),
]

MY_GRID_DAMASK: list[tuple[str, Quantity]] = [
    ("extent", ureg.Quantity(np.asarray([128, 52, 64], np.uint32))),
    ("cell_dimensions", ureg.Quantity(np.float64(0.4982421), ureg.micrometer)),
]

MY_SIM_CONTROL: list[tuple[str, Quantity]] = [
    ("max_x", ureg.Quantity(np.float64(1.0))),
    ("max_time", ureg.Quantity(np.float64(3600.0), ureg.second)),
    ("max_iteration", ureg.Quantity(np.uint32(10000))),
    ("max_delta_x", ureg.Quantity(np.float64(0.1))),
    (
        "x_set",
        ureg.Quantity(
            np.asarray(np.linspace(0.0, stop=1.0, num=101, endpoint=True), np.float64)
        ),
    ),
]
