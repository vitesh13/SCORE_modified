# SPDX-License-Identifier: GNU General Public License v3.0 or later

from typing import Any

import numpy as np
from score_config_utils import ureg

# !!! DO NOT OVERWRITE PARAMETER HERE BUT DO SO IN YOUR OWN SCRIPT !!!
# !!! SEE THE EXAMPLE IN NXmicrostructure_score_config.ipynb !!!

# DF is an abbreviation for DEFAULT PARAMETER
DF_MATERIAL: list[tuple[str, Any]] = [
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

DF_INIT_CUBOIDAL: dict[str, Any] = {
    "extent": ureg.Quantity(
        np.asarray([30.0, 30.0, 30.0], np.float64), ureg.micrometer
    ),
    "bunge_euler": ureg.Quantity(
        np.asarray([[60.0, 55.0, 45.0], [60.0, 55.0, 45.0]], np.float64), ureg.degree
    ).to(ureg.radian),
    "stored_energy": ureg.Quantity(
        np.asarray([1.0e15, 0.5e15], np.float64), ureg.meter**-2
    ),
}

DF_INIT_DAMASK: dict[str, Any] = {
    "file_path": "Tphase_Vf_01_tensionX_remeshed_increment19000_segmented.dream3D"
}

DF_NUCLEATION_CSR: dict[str, Any] = {
    "bunge_euler": ureg.Quantity(
        np.asarray([[0.0, 30.0, 45.0]], np.float64), ureg.degree
    ).to(ureg.radian),
    "incubation_time": ureg.Quantity(np.asarray([0.0], np.float64), ureg.second),
}

DF_MOBILITY_RH: list[tuple[str, Any]] = [
    (
        "m_null",
        ureg.Quantity(np.float64(300.0), ureg.meter**4 / (ureg.joule * ureg.second)),
    ),
    ("enthalpy", ureg.Quantity(np.float64(2.0314), ureg.eV)),
    ("c1", ureg.Quantity(np.float64(0.9))),
    ("c2", ureg.Quantity(np.float64(5.0))),
    ("c3", ureg.Quantity(np.float64(9.0))),
]

DF_MOBILITY_SG: list[tuple[str, Any]] = [
    ("lagb_to_hagb_cut", ureg.Quantity(np.float64(15.0), ureg.degree).to(ureg.radian)),
    (
        "lagb_pre_factor",
        ureg.Quantity(np.float64(30.0), ureg.meter**4 / (ureg.joule * ureg.second)),
    ),
    ("lagb_enthalpy", ureg.Quantity(np.float64(2.45), ureg.eV)),
    (
        "hagb_pre_factor",
        ureg.Quantity(np.float64(30.0), ureg.meter**4 / (ureg.joule * ureg.second)),
    ),
    ("hagb_enthalpy", ureg.Quantity(np.float64(2.45), ureg.eV)),
    (
        "special_disorientation",
        ureg.Quantity(np.float64(10.0), ureg.degree).to(ureg.radian),
    ),
    (
        "special_pre_factor",
        ureg.Quantity(np.float64(30.0), ureg.meter**4 / (ureg.joule * ureg.second)),
    ),
    ("special_enthalpy", ureg.Quantity(np.float64(2.45), ureg.eV)),
]

DF_TEXTURE: dict[str, Any] = {
    "name": ["Alpha001-110", "Alpha112-110"],
    "bunge_euler": ureg.Quantity(
        np.asarray([[0.0, 0.0, 0.0], [0.0, 35.0, 45.0]], np.float64), ureg.degree
    ).to(ureg.radian),
    "theta": ureg.Quantity(np.asarray([10.0, 10.0], np.float64), ureg.degree).to(
        ureg.radian
    ),
}

DF_TIME_TEMPERATURE: list[tuple[str, Any]] = [
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

DF_GRID_CUBOIDAL: list[tuple[str, Any]] = [
    ("extent", ureg.Quantity(np.asarray([19, 15, 11], np.uint32))),
    ("cell_dimensions", ureg.Quantity(np.float64(0.5), ureg.micrometer)),
]

DF_GRID_DAMASK: list[tuple[str, Any]] = [
    ("extent", ureg.Quantity(np.asarray([128, 52, 64], np.uint32))),
    ("cell_dimensions", ureg.Quantity(np.float64(0.4982421), ureg.micrometer)),
]

DF_SIM_CONTROL: list[tuple[str, Any]] = [
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
