# SPDX-License-Identifier: GPL-3.0-or-later

"""Default parameterization of the SCORE model."""

from typing import Any

import numpy as np
from score_config_base import ConfigBase
from score_config_default_parameter import (
    DF_GRID_CUBOIDAL,
    DF_INIT_CUBOIDAL,
    DF_MATERIAL,
    DF_MOBILITY_RH,
    DF_NUCLEATION_CSR,
    DF_SIM_CONTROL,
    DF_TEXTURE,
    DF_TIME_TEMPERATURE,
)
from score_config_utils import get_nexus_version, get_sha256_of_file_content, ureg


class ScoreConfig(ConfigBase):
    # attributes use @ as a marker/escape character
    # groups are all those concepts which have an attribute @NX_class
    # fields are all those other concepts with an attribute %value
    def __init__(self, simid: int = 0):
        self.deg2kelvin = 273.15
        self.sim_id = simid
        self.entry_id = 1
        self.yaml_file_name = f"NXmicrostructure_score_config.SimID.{self.sim_id}.yaml"
        self.hdf5_file_name = f"SCORE.Config.SimID.{self.sim_id}.nxs"
        self.cfg: dict[str, Any] = {}
        self.cfg["@NX_class"] = "NXroot"
        self.cfg["@default"] = f"entry{self.entry_id}"
        self.cfg[f"entry{self.entry_id}/@NX_class"] = "NXentry"
        self.cfg[f"entry{self.entry_id}/@default"] = "time_temperature"
        self.cfg[f"entry{self.entry_id}/definition"] = "NXmicrostructure_score_config"
        self.cfg[f"entry{self.entry_id}/definition/@version"] = f"{get_nexus_version()}"
        # "simulation_identifier": None,
        # "start_time": None,
        # "end_time": None,
        # "program1": None
        # physics
        # self.description()
        self.user()
        self.sample()
        self.material()
        # self.deformation()
        # self.nucleation()
        self.mobility()
        self.recovery()
        self.drag()
        self.component_analysis()
        self.time_temperature()
        # numerics
        self.discretization()
        self.numerics()
        self.solitary_unit_model()

    def description(self):
        """Configure ELN-like description."""
        trg = f"entry{self.entry_id}/"
        self.cfg[f"{trg}description"] = (
            "Example showing how to bring recrystallization simulations with SCORE into NOMAD."
            ""
        )

    def user(self, users: list[tuple[str, str]] = [("SCORE User", "")]):
        """Configure the user."""
        idx = 0
        for user in users:
            if isinstance(user, tuple) and len(user) == 2 and user[0] != "":
                trg = f"entry{self.entry_id}/user{idx + 1}/"
                self.cfg[f"{trg}@NX_class"] = "NXuser"
                self.cfg[f"{trg}name"] = user[0]
                if user[1] != "":
                    self.cfg[f"{trg}ORCID"] = user[1]
                idx += 1

    def sample(self, atom_types: str = "Al"):
        """Configure contextualizing details about the sample."""
        trg = f"entry{self.entry_id}/sample/"
        self.cfg[f"{trg}@NX_class"] = "NXsample"
        self.cfg[f"{trg}dimensionality"] = "3"
        self.cfg[f"{trg}is_simulation"] = True
        self.cfg[f"{trg}atom_types"] = atom_types

    def material(self, parameters: list[tuple] = DF_MATERIAL):
        """Configure material model single-phase only."""
        trg = f"entry{self.entry_id}/material/"
        self.cfg[f"{trg}@NX_class"] = "NXparameters"
        for key, quantity in parameters:
            self.cfg[f"{trg}{key}"] = quantity

    def deformation(
        self, model: str = "cuboidal", parameters: dict[str, Any] = DF_INIT_CUBOIDAL
    ):
        """Configure deformed (initial) microstructure."""
        if model not in ("cuboidal", "damask"):
            return
        trg = f"entry{self.entry_id}/deformation/"
        self.cfg[f"{trg}@NX_class"] = "NXparameters"
        self.cfg[f"{trg}model"] = model
        if model == "cuboidal":
            if "extent" in parameters:
                self.cfg[f"{trg}extent"] = parameters["extent"]
            if all(val in parameters for val in ("bunge_euler", "stored_energy")):
                if (
                    np.shape(parameters["bunge_euler"].magnitude)[0]
                    == np.shape(parameters["stored_energy"])[0]
                ):
                    self.cfg[f"{trg}ensemble/@NX_class"] = "NXparameters"
                    for key in ("bunge_euler", "stored_energy"):
                        self.cfg[f"{trg}ensemble/{key}"] = parameters[key]
        elif model == "damask":
            if "file_path" in parameters:
                abs_file_path = parameters["file_path"]
                if abs_file_path:
                    self.cfg[f"{trg}damask/@NX_class"] = "NXnote"
                    self.cfg[f"{trg}damask/file_name"] = f"{abs_file_path}"
                    self.cfg[f"{trg}damask/algorithm"] = "sha256"
                    self.cfg[f"{trg}damask/checksum"] = get_sha256_of_file_content(
                        abs_file_path
                    )

    def nucleation(self, model="csr", parameters: dict[str, Any] = DF_NUCLEATION_CSR):
        """Configure nucleation model."""
        if model not in ("csr", "damask", "continuous"):
            return
        trg = f"entry{self.entry_id}/nucleation/"
        self.cfg[f"{trg}@NX_class"] = "NXparameters"
        self.cfg[f"{trg}spatial_distribution"] = model
        self.cfg[f"{trg}number_of_nuclei"] = ureg.Quantity(np.uint32(1))
        self.cfg[f"{trg}incubation_time"] = "site_saturation"
        # self.cfg[f"{trg}incubation_time_sigma"] = ureg.Quantity(np.float64(1000.), ureg.second)
        if model == "damask":
            self.cfg[f"{trg}orientation"] = "damask"
        elif model == "continuous":
            self.cfg[f"{trg}orientation"] = "damask"
            self.cfg[f"{trg}incubation_time"] = "continuous"
            for key in ("activation_energy", "pre_factor", "critical_dislocation_density"):
                self.cfg[f"{trg}{key}"] = parameters[key]
        else:  # "csr":
            self.cfg[f"{trg}orientation"] = "ensemble"  # "random"
            if "bunge_euler" in parameters and "incubation_time" in parameters:
                if (
                    np.shape(parameters["bunge_euler"])[0]
                    == np.shape(parameters["incubation_time"])[0]
                ):
                    self.cfg[f"{trg}ensemble/@NX_class"] = "NXparameters"
                    for key in ("bunge_euler", "incubation_time"):
                        self.cfg[f"{trg}ensemble/{key}"] = parameters[key]

    def mobility(
        self, model="rollett_holm", parameters: list[tuple[str, Any]] = DF_MOBILITY_RH
    ):
        """Configure grain boundary mobility model."""
        if model not in ("sebald_gottstein", "rollett_holm"):
            return
        trg = f"entry{self.entry_id}/grain_boundary_mobility/"
        self.cfg[f"{trg}@NX_class"] = "NXparameters"
        self.cfg[f"{trg}@NX_class"] = "NXparameters"
        self.cfg[f"{trg}model"] = model
        trg += f"{model}/"
        self.cfg[f"{trg}@NX_class"] = "NXparameters"
        for key, quantity in parameters:
            self.cfg[f"{trg}{key}"] = quantity

    def recovery(self):
        """Configure stored elastic energy recovery model."""
        trg = f"entry{self.entry_id}/stored_energy_recovery/"
        self.cfg[f"{trg}@NX_class"] = "NXparameters"
        self.cfg[f"{trg}model"] = "none"

    def drag(self):
        """Configure reduction of grain boundary mobility by dispersoids (Zener-Smith drag)."""
        trg = f"entry{self.entry_id}/dispersoid_drag/"
        self.cfg[f"{trg}@NX_class"] = "NXdata"
        self.cfg[f"{trg}model"] = "none"
        # trg = "entry{self.entry_id}/dispersoid_drag/zener_smith/"
        # self.cfg[f"{trg}@NX_class"] = "NXparameters"
        # self.cfg[f"{trg}@signal"] = "radius"
        # self.cfg[f"{trg}@axes"] = "time"
        # self.cfg[f"{trg}@time_indices"] = ureg.Quantity(np.uint32(0))
        # self.cfg[f"{trg}@radius_indices"] = ureg.Quantity(np.uint32(1))
        # self.cfg[f"{trg}title"] = "r(t)"
        # self.cfg[f"{trg}pre_factor"] = ureg.Quantity(np.float64(1.5))
        # self.cfg[f"{trg}surface_energy"] = ureg.Quantity(np.float64(0.324), ureg.joule / (ureg.meter ** 2))
        # self.cfg[f"{trg}time/@long_name"] = "t (s)"
        # self.cfg[f"{trg}radius/@long_name"] = "r (nm)"
        # self.cfg[f"{trg}time"] = ureg.Quantity(np.asarray([0., 3600.], np.float64), ureg.second)
        # self.cfg[f"{trg}radius"] = ureg.Quantity(np.asarray([0., 0.], np.float64), ureg.nanometer)

    def component_analysis(self, parameters: dict[str, Any] = DF_TEXTURE):
        """Configure which macrotexture components the code should bookkeep for volume evolution."""
        trg = f"entry{self.entry_id}/component_analysis/"
        self.cfg[f"{trg}@NX_class"] = "NXparameters"
        if all([key in parameters for key in ("names", "bunge_euler", "theta")]):
            num = np.shape(parameters["names"])[0]
            if all(
                np.shape(parameters[val])[0] == num for val in ("bunge_euler", "theta")
            ):
                for key, quantity in parameters.items():
                    self.cfg[f"{trg}{key}"] = quantity

    def time_temperature(self, parameters: list[tuple] = DF_TIME_TEMPERATURE):
        """Configure the simulated annealing schedule."""
        trg = f"entry{self.entry_id}/time_temperature/"
        self.cfg[f"{trg}@NX_class"] = "NXdata"
        # annotations for generating default plots using H5Web
        self.cfg[f"{trg}@signal"] = "temperature"
        self.cfg[f"{trg}@axes"] = "time"
        self.cfg[f"{trg}@time_indices"] = ureg.Quantity(np.uint32(0))
        self.cfg[f"{trg}@temperature_indices"] = ureg.Quantity(np.uint32(1))
        self.cfg[f"{trg}title"] = "T(t)"
        self.cfg[f"{trg}time/@long_name"] = "t (s)"
        self.cfg[f"{trg}temperature/@long_name"] = "T (K)"
        for key, quantity in parameters:
            self.cfg[f"{trg}{key}"] = quantity

    def discretization(self, parameters: list[tuple] = DF_GRID_CUBOIDAL):
        """Configure microstructure instantiation."""
        trg = f"entry{self.entry_id}/discretization/"
        self.cfg[f"{trg}@NX_class"] = "NXmicrostructure"
        trg += "grid/"
        self.cfg[f"{trg}@NX_class"] = "NXcg_grid"
        for key, quantity in parameters:
            self.cfg[f"{trg}{key}"] = quantity

    def numerics(self, parameters: list[tuple] = DF_SIM_CONTROL):
        """Configure stop criteria and numerics."""
        trg = f"entry{self.entry_id}/numerics/"
        self.cfg[f"{trg}@NX_class"] = "NXparameters"
        for key, quantity in parameters:
            self.cfg[f"{trg}{key}"] = quantity
        # self.cfg[f"{trg}percolation"] = False
        # battle-proven default values follow, see Kuehbach2017 for description about cell defragmentation
        self.cfg[f"{trg}cell_cache/@NX_class"] = "NXparameters"
        self.cfg[f"{trg}cell_cache/initial"] = ureg.Quantity(np.float64(0.2))
        self.cfg[f"{trg}cell_cache/realloc"] = ureg.Quantity(np.float64(1.0))
        self.cfg[f"{trg}cell_cache/defragment"] = True
        self.cfg[f"{trg}cell_cache/defragment_x"] = ureg.Quantity(
            np.asarray(
                [
                    0.60,
                    0.65,
                    0.70,
                    0.75,
                    0.80,
                    0.85,
                    0.90,
                    0.92,
                    0.94,
                    0.96,
                    0.98,
                    0.99,
                    0.999,
                    0.9999,
                ],
                np.float64,
            )
        )

    def solitary_unit_model(self, number_of_domains: int = 1):
        """Configure solitary unit post-processing approach."""
        trg = f"entry{self.entry_id}/solitary_unit/"
        self.cfg[f"{trg}@NX_class"] = "NXparameters"
        if number_of_domains < 1:
            self.cfg[f"{trg}apply"] = False
            self.cfg[f"{trg}number_of_domains"] = ureg.Quantity(np.uint32(1))
        elif number_of_domains >= 1:
            self.cfg[f"{trg}apply"] = True
            self.cfg[f"{trg}number_of_domains"] = ureg.Quantity(
                np.uint32(number_of_domains)
            )
        self.cfg[f"{trg}rediscretization"] = ureg.Quantity(np.uint32(10000))
        # self.cfg[f"{trg}domain_identifier"] = None
