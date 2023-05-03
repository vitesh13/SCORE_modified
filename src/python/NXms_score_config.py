
def initial_microstructure(self, domain_size=[10, 10, 10], type="ebsd", kwargs(cell_size, grain_size, grain_diameter, grain_euler, ebsd_filename)):
    """Configure microstructure instantiation."""

def nucleation_model(self,
                     spatial_distribution_model="csr",
                     incubation_time_model="site_saturation",
                     orientation_model="sample_from_nucleus_euler"):
    """Configure nucleation model."""
    

def material_properties(self,
                        reference_shear_modulus=79.0e9,
                        reference_burgers_magnitude=256.0e-12,
                        melting_temperature=600.0):
    "Configure material model"""
    trg = "material_properties/"
    self.cfg[f"{trg}reference_shear_modulus"] = (9.5e9, "Pa")
    self.cfg[f"{trg}reference_burgers_magnitude"] = (286.5e-12, "m")
    self.cfg[f"{trg}melting_temperature"] = (800., "°C")

def mobility_model(self,
                   model="rollett_holm"):
    """Configure mobility model."""
    if model == "sebald_gottstein":
        trg = "grain_boundary_mobility_model/"
        self.cfg[f"{trg}model"] = "sebald_gottstein"
        trg = "grain_boundary_mobility_model/sebald_gottstein_parameters/"
        self.cfg[f"{trg}lagb_pre_factor"] = (300., "m^4/(Js)")
        self.cfg[f"{trg}lagb_enthalpy"] = (2.45, "eV")
        self.cfg[f"{trg}lagb_to_hagb_transition"] = (15., "°")
        self.cfg[f"{trg}hagb_pre_factor"] = (300., "m^4/(Js)")
        self.cfg[f"{trg}hagb_enthalpy"] = (2.45, "eV")
        self.cfg[f"{trg}special_pre_factor"] = (300., "m^4/(Js)")
        self.cfg[f"{trg}special_enthalpy"] = (2.45, "eV")
        self.cfg[f"{trg}special_disorientation"] = (10., "°") MAXDISORI_TO_40DEG111

    if model == "rollett_holm":
        trg = "grain_boundary_mobility_model/"
        self.cfg[f"{trg}model"] = "rollett_holm"
        trg = "grain_boundary_mobility_model/rollett_holm_parameters/"
        self.cfg[f"{trg}hagb_pre_factor"] = (300., "m^4/(Js)")
        self.cfg[f"{trg}hagb_enthalpy"] = (1., "eV")
        self.cfg[f"{trg}lagb_to_hagb_cut"] = (0.9, "")
        self.cfg[f"{trg}lagb_to_hagb_transition"] = (5., "")
        self.cfg[f"{trg}lagb_to_hagb_exponent"] = (9., "")

def recovery_model(self, model="none"):
    """Configure stored elastic energy recovery model."""
    trg = "stored_energy_recovery_model/"
    self.cfg[f"{trg}model"] = "none"

def drag_model(self, model="none", zener_smith_parameter):
    """Configure reduction of grain boundary mobility by dispersoids."""
    trg = "dispersoid_drag_model/"
    self.cfg[f"{trg}model"] = "none"
    trg = "dispersoid_drag_model/zener_smith_parameter/"
    self.cfg[f"{trg}prefactor"] = (1.5, "")
    self.cfg[f"{trg}surface_energy_density"] = (0.324, "J/m^2")
    self.cfg[f"{trg}time"] = (np.asarray([0., 3600.], np.float64), "s")
    self.cfg[f"{trg}radius"] = (np.asarray([0., 0.], np.float64), "nm")

def time_temperature_history(self):
    """Configure the simulated annealing schedule."""
    self.cfg["time_temperature_history/time"] = (np.asarray(
0., 1., 2., 3., 4., 5., 6., 7., 8., 9., 10.,
11., 12., 16., 20., 21., 22., 23., 24., 25.,
26., 27., 28., 29., 30., 31., 32., 36., 40.,
41., 42., 43., 44., 45., 46., 47., 48., 49.,
50., 51., 52., 53., 54., 55., 56., 57., 58.,
3600.], np.float64), "s")
    self.cfg["time_temperature_history/temperature"] = (np.asarray(
20., 165., 310., 455., 600., 600., 600., 600., 600., 455., 310., 165., 20., 20., 20., 165., 310., 455., 600., 600., 600., 600., 600., 455., 310., 165., 20., 20., 20., 165., 310., 455., 600., 600., 600., 600., 600., 600., 600., 600., 600., 600., 600., 455., 310., 165., 20., 20.], np.float64), "°C")

# ideal components

def numerics(self,
             max_x=(1., ""),
             max_time=(3600., "s"),
             max_iteration=(10000, "")):
    """Configure stop criteria and numerics."""
    max_delta_x (0.1, "")
    initial_cell_cache (0.2, "")
    realloc_cell_cache (1.0, "")
    defragment_cell_cache=True
    defragment_x=np.asarray([0.60, 0.65, 0.70, 0.75, 0.80, 0.85, 0.90,
                  0.92, 0.94, 0.96, 0.98, 0.99, 0.999, 0.9999], np.float64)
    snapshot_x=np.asarray(np.linspace(0.000, stop=1.000, num=241, endpoint=True), np.float64)
    percolation=False
    # value, unit tuple

def solitary_unit_model(self, apply=False, number_of_domains=1, kwargs
    rediscretization=10000,
    domain_identifier=
    
