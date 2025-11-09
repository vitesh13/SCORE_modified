# SPDX-License-Identifier: GNU General Public License v3.0 or later

import numpy as np
from score_config_utils import ureg
from typing import List, Tuple, Dict, Any


#### DF_ means DEFAULT PARAMETER VALUES
#### meant to be overwritten by MY_ for specific scientific simulation

#### !!! DO NOT OVERWRITE HERE BUT AS SHOWN BUT IN YOUR OWN SCRIPT !!!
#### !!! AS SHOWN BY NXmicrostructure_score_config.ipynb !!!

DF_MATERIAL: List[Tuple[str, Any]] = [
    ("shear_modulus", ureg.Quantity(np.float64(9.5e9), ureg.pascal)),
    ("burgers_vector", ureg.Quantity(np.float64(0.286), ureg.nanometer)),
]

DF_INIT_CUBOIDAL: Dict[str, Any] = {
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

DF_INIT_DAMASK: Dict[str, Any] = {
    "file_path": "Tphase_Vf_01_tensionX_remeshed_increment19000_segmented.dream3D"
}

DF_NUCLEATION_CSR: Dict[str, Any] = {
    "bunge_euler": ureg.Quantity(
        np.asarray([[0.0, 30.0, 45.0]], np.float64), ureg.degree
    ).to(ureg.radian),
    "incubation_time": ureg.Quantity(np.asarray([0.0], np.float64), ureg.second),
}

DF_MOBILITY_RH: List[Tuple[str, Any]] = [
    (
        "m_null",
        ureg.Quantity(np.float64(300.0), ureg.meter**4 / (ureg.joule * ureg.second)),
    ),
    ("enthalpy", ureg.Quantity(np.float64(2.0314), ureg.eV)),
    ("c1", ureg.Quantity(np.float64(0.9))),
    ("c2", ureg.Quantity(np.float64(5.0))),
    ("c3", ureg.Quantity(np.float64(9.0))),
]

DF_MOBILITY_SG: List[Tuple[str, Any]] = [
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

DF_TEXTURE: Dict[str, Any] = {
    "name": ["Alpha001-110", "Alpha112-110"],
    "bunge_euler": ureg.Quantity(
        np.asarray([[0.0, 0.0, 0.0], [0.0, 35.0, 45.0]], np.float64), ureg.degree
    ).to(ureg.radian),
    "theta": ureg.Quantity(np.asarray([10.0, 10.0], np.float64), ureg.degree).to(
        ureg.radian
    ),
}

DF_TIME_TEMPERATURE: List[Tuple[str, Any]] = [
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

DF_GRID_CUBOIDAL: List[Tuple[str, Any]] = [
    ("extent", ureg.Quantity(np.asarray([19, 15, 11], np.uint32))),
    ("cell_dimensions", ureg.Quantity(np.float64(0.5), ureg.micrometer)),
]

DF_GRID_DAMASK: List[Tuple[str, Any]] = [
    ("extent", ureg.Quantity(np.asarray([128, 52, 64], np.uint32))),
    ("cell_dimensions", ureg.Quantity(np.float64(0.4982421), ureg.micrometer)),
]

DF_SIM_CONTROL: List[Tuple[str, Any]] = [
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
