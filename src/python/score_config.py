"""Default parameterization of the SCORE model."""

import numpy as np
from score_config_base import ConfigBase
from score_config_utils import get_nexus_version, get_sha256_of_file_content, ureg
from typing import Any, Dict, List, Tuple
from score_config_default_parameter import (
    DF_MATERIAL,
    DF_INIT_CUBOIDAL,
    DF_INIT_DAMASK,
    DF_NUCLEATION_CSR,
    DF_MOBILITY_RH,
    DF_MOBILITY_SG,
    DF_TEXTURE,
    DF_TIME_TEMPERATURE,
    DF_GRID_CUBOIDAL,
    DF_GRID_DAMASK,
    DF_SIM_CONTROL,
)


class ScoreConfig(ConfigBase):
    # attributes use @ as a marker/escape character
    # groups are all those concepts which have an attribute @NX_class
    # fields are all those other concepts with an attribute %value
    def __init__(self, simid: int):
        self.deg2kelvin = 273.15
        self.sim_id = simid
        self.entry_id = 1
        self.yaml_file_name = f"NXmicrostructure_score_config.SimID.{self.sim_id}.yaml"
        self.hdf5_file_name = f"SCORE.Config.SimID.{self.sim_id}.nxs"
        self.cfg: Dict[str, Any] = {}
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
        self.deformation()
        self.nucleation()
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

    def user(self, user_names: List[str] = ["SCORE User"]):
        """Configure the user."""
        idx = 0
        for user_name in user_names:
            if user_name != "":
                trg = f"entry{self.entry_id}/user{idx + 1}/"
                self.cfg[f"{trg}@NX_class"] = "NXuser"
                self.cfg[f"{trg}name"] = user_name
                idx += 1

    def sample(self, atom_types: str = "Al"):
        """Configure contextualizing details about the sample."""
        trg = f"entry{self.entry_id}/sample/"
        self.cfg[f"{trg}@NX_class"] = "NXsample"
        self.cfg[f"{trg}dimensionality"] = ureg.Quantity(np.uint32(3))
        self.cfg[f"{trg}type"] = "simulation"
        self.cfg[f"{trg}atom_types"] = atom_types

    def material(self, parms: List[Tuple] = DF_MATERIAL):
        """Configure material model single-phase only."""
        trg = f"entry{self.entry_id}/material/"
        self.cfg[f"{trg}@NX_class"] = "NXparameters"
        for parm, qnt in parms:
            self.cfg[f"{trg}{parm}"] = qnt

    def deformation(self, model: str = "cuboidal", parms: Tuple = DF_INIT_CUBOIDAL):
        """Configure deformed (initial) microstructure."""
        if model not in ("cuboidal", "damask"):
            return
        trg = f"entry{self.entry_id}/deformation/"
        self.cfg[f"{trg}@NX_class"] = "NXparameters"
        self.cfg[f"{trg}model"] = model
        if model == "cuboidal":
            if "extent" in parms:
                self.cfg[f"{trg}extent"] = parms["extent"]
            if all(val in parms for val in ("bunge_euler", "stored_energy")):
                if (
                    np.shape(parms["bunge_euler"].magnitude)[0]
                    == np.shape(parms["stored_energy"])[0]
                ):
                    self.cfg[f"{trg}ensemble/@NX_class"] = "NXparameters"
                    for parm in ("bunge_euler", "stored_energy"):
                        self.cfg[f"{trg}ensemble/{parm}"] = parms[parm]
        elif model == "damask":
            if "file_path" in parms:
                abs_file_path = parms["file_path"]
                if abs_file_path:
                    self.cfg[f"{trg}damask/@NX_class"] = "NXnote"
                    self.cfg[f"{trg}damask/file_name"] = f"{abs_file_path}"
                    self.cfg[f"{trg}damask/algorithm"] = "sha256"
                    self.cfg[f"{trg}damask/checksum"] = get_sha256_of_file_content(
                        abs_file_path
                    )

    def nucleation(self, model="csr", parms=DF_NUCLEATION_CSR):
        """Configure nucleation model."""
        if model not in ("csr", "damask"):
            return
        trg = f"entry{self.entry_id}/nucleation/"
        self.cfg[f"{trg}@NX_class"] = "NXparameters"
        self.cfg[f"{trg}spatial_distribution"] = model
        self.cfg[f"{trg}number_of_nuclei"] = ureg.Quantity(np.uint32(1))
        self.cfg[f"{trg}incubation_time"] = "site_saturation"
        # self.cfg[f"{trg}incubation_time_sigma"] = ureg.Quantity(np.float64(1000.), ureg.second)
        if model == "damask":
            self.cfg[f"{trg}orientation"] = "damask"
        else:  # "csr":
            self.cfg[f"{trg}orientation"] = "sample_from_nucleus_euler"
            if "bunge_euler" in parms and "incubation_time" in parms:
                if (
                    np.shape(parms["bunge_euler"])[0]
                    == np.shape(parms["incubation_time"])[0]
                ):
                    self.cfg[f"{trg}ensemble/@NX_class"] = "NXparameters"
                    for parm in ("bunge_euler", "incubation_time"):
                        self.cfg[f"{trg}ensemble/{parm}"] = parms[parm]

    def mobility(self, model="rollett_holm", parms: List[Tuple] = DF_MOBILITY_RH):
        """Configure grain boundary mobility model."""
        if model not in ("sebald_gottstein", "rollett_holm"):
            return
        trg = f"entry{self.entry_id}/grain_boundary_mobility/"
        self.cfg[f"{trg}@NX_class"] = "NXparameters"
        self.cfg[f"{trg}@NX_class"] = "NXparameters"
        self.cfg[f"{trg}model"] = model
        trg += f"{model}/"
        self.cfg[f"{trg}@NX_class"] = "NXparameters"
        for parm, qnt in parms:
            self.cfg[f"{trg}{parm}"] = qnt

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

    def component_analysis(self, parms=DF_TEXTURE):
        """Configure which macrotexture components the code should bookkeep for volume evolution."""
        trg = f"entry{self.entry_id}/component_analysis/"
        self.cfg[f"{trg}@NX_class"] = "NXparameters"
        if all(val in parms for val in ("name", "bunge_euler", "theta")):
            num = np.shape(parms["name"])[0]
            if all(np.shape(parms[val])[0] == num for val in ("bunge_euler", "theta")):
                for parm in parms:
                    self.cfg[f"{trg}{parm}"] = parms[parm]

    def time_temperature(self, parms: List[Tuple] = DF_TIME_TEMPERATURE):
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
        for parm, qnt in parms:
            self.cfg[f"{trg}{parm}"] = qnt

    def discretization(self, parms: List[Tuple] = DF_GRID_CUBOIDAL):
        """Configure microstructure instantiation."""
        trg = f"entry{self.entry_id}/discretization/"
        self.cfg[f"{trg}@NX_class"] = "NXmicrostructure"
        trg += "grid/"
        self.cfg[f"{trg}@NX_class"] = "NXcg_grid"
        for parm, qnt in parms:
            self.cfg[f"{trg}{parm}"] = qnt

    def numerics(self, parms: List[Tuple] = DF_SIM_CONTROL):
        """Configure stop criteria and numerics."""
        trg = f"entry{self.entry_id}/numerics/"
        self.cfg[f"{trg}@NX_class"] = "NXparameters"
        for parm, qnt in parms:
            self.cfg[f"{trg}{parm}"] = qnt
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
        if number_of_domains <= 1:
            self.cfg[f"{trg}apply"] = False
            self.cfg[f"{trg}number_of_domains"] = ureg.Quantity(np.uint32(1))
        elif number_of_domains >= 1:
            self.cfg[f"{trg}apply"] = True
            self.cfg[f"{trg}number_of_domains"] = ureg.Quantity(
                np.uint32(number_of_domains)
            )
        self.cfg[f"{trg}rediscretization"] = ureg.Quantity(np.uint32(10000))
        # self.cfg[f"{trg}domain_identifier"] = None
