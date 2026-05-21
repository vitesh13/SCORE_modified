import glob
import os

import damask
import h5py
import numpy as np
from damask import GeomGrid, Rotation
from scipy.spatial import cKDTree

Crystal_structures = {"fcc": 1, "bcc": 1, "hcp": 0, "bct": 7, "ort": 6}
# todo: some of them do not have the support anymore it seems
Phase_types = {
    "Primary": 0
}  # further additions can be done if needed by looking at 'Create Ensemble Info' filter.


class AttributeManagerNullterm(h5py.AttributeManager):
    """
    Attribute management for DREAM.3D hdf5 files.

    String attribute values are stored as fixed-length string with NULLTERM

    References
    ----------
      https://stackoverflow.com/questions/38267076
      https://stackoverflow.com/questions/52750232
    """

    def create(self, name, data, shape=None, dtype=None):
        if isinstance(data, str):
            tid = h5py.h5t.C_S1.copy()
            tid.set_size(len(data + " "))
            super().create(name=name, data=data + " ", dtype=h5py.Datatype(tid))
        else:
            super().create(name=name, data=data, shape=shape, dtype=dtype)


h5py._hl.attrs.AttributeManager = AttributeManagerNullterm  # 'Monkey patch'


class DAMASKtoDREAM3D:
    """
    This class can convert the DAMASK data to DREAM3D compatible data.
    There can be various different types of ways DAMASK data can be represented.
    Therefore, there are multiple functions available for different purposes.
    """

    def __init__(self, simulation_folder, job_file, geom_file, load_file):
        """
        Defining the common quantities for all the functions in this class.

        Parameters
        ----------
        simulation_folder: str
            Path of the simulation folder.
        job_file: str
            Name of the job file (DADF5 file).
        geom_file : str
          name of the geom file.
        load_file :
          name of the load file.
        """
        self.simulation_folder = simulation_folder
        self.job_file = job_file
        self.geom_file = geom_file
        self.load_file = load_file

    def get_regridded_inc_list(self):
        """Gives a sorted list of regridding increments."""
        regridded_file_list = glob.glob(
            f"{self.geom_file}_regridded_increment*.vti", recursive=True
        )
        regridded_inc_list = [os.path.splitext(i)[0] for i in regridded_file_list]
        regridded_inc_list = [
            int(i.split(f"{self.geom_file}_regridded_increment_")[1])
            for i in regridded_inc_list
        ]

        regridded_inc_list.sort()
        self.regridded_inc_list = regridded_inc_list

    def get_correct_regridding_index(self, inc):
        """Gives correct regridding index based on the increment being asked for."""
        index = 0
        for i in self.regridded_inc_list:
            if i <= inc:
                index = index + 1
        return index - 1 if index > 0 else index

    def regrid_datasets_scalar(self, inc, dataset):
        """
        Regrids the data by reading the dataset in the results file.
        Currently only works for scalar datasets.

        Parameters:
        -----------
        inc: int
          increment of interest for DREAM3D processing.
        dataset: str
          Name of the dataset to be regridded.
        """
        os.chdir(self.simulation_folder)
        # --------------------------------------------------------------------------
        # Build array of euler angles for each cell
        # --------------------------------------------------------------------------
        d = damask.Result(self.job_file)
        inc_data = d.view(
            increments=inc
        )  # selecting only relevant data to reduce overload

        map_file = h5py.File("regridding.hdf5", "r")
        regridding_steps = [int(i) for i in map_file["phase"].keys()]
        regridding_steps.sort()

        self.get_regridded_inc_list()
        regridding_index = self.get_correct_regridding_index(inc)

        f = h5py.File(self.job_file, "r")
        geom = GeomGrid.load(
            f"{self.geom_file}_regridded_increment_{self.regridded_inc_list[regridding_index]}.vti"
        )
        cells_rg = geom.cells
        size_rg = geom.size

        # regrid the geometry (but at the same dimensions as before)
        map_0to_rg = np.array(map_file[f"map/{regridding_steps[regridding_index]}"])
        print(regridding_index)

        # original dataset array
        data_dict = inc_data.get(f"{dataset}")

        # construct the original cell orientation array and then regrid it
        data_array = np.zeros(np.prod(cells_rg))

        # construct the original phase array and then regrid it
        phase_ID_array = np.zeros(
            (np.prod(cells_rg)), dtype=np.int32
        )  # need to reshape it later

        # based on the regridding step, find the correct map array with phases
        if regridding_index == 0:
            map_0toRg_phaseBased = f["cell_to/phase"]  # map before the regridding
            map_0toRg_phaseBased = map_0toRg_phaseBased[:, 0][map_0to_rg]
        else:
            map_0toRg_phaseBased = map_file[
                f"phase/{regridding_steps[regridding_index - 1]}"
            ]  # map corresponding to a step before
            map_0toRg_phaseBased_reshaped = np.array(map_0toRg_phaseBased).reshape(
                (len(map_0toRg_phaseBased), 1)
            )
            map_0toRg_phaseBased = map_0toRg_phaseBased_reshaped[:, 0][map_0to_rg]

        # create the original arrays
        for count, p in enumerate(d.phases):
            phase_index = map_0toRg_phaseBased[
                map_0toRg_phaseBased["label"] == f"{p}".encode()
            ]["entry"]
            map_index = np.where(map_0toRg_phaseBased["label"] == f"{p}".encode())[0]
            if len(d.phases) > 1:
                if f"{dataset}" in f[f"/increment_{inc}/phase/{p}/mechanical"].keys():
                    try:
                        data_dict.keys()
                        data_array[map_index] = data_dict[f"{p}"][phase_index]
                    except AttributeError:
                        data_array[map_index] = data_dict[phase_index]
                else:
                    continue
            else:
                data_array[map_index] = data_dict[phase_index]

            phase_ID_array[map_index] = count + 1

        return data_array

    def get_remesh_map(self, inc):
        """
        Gives a map for remeshing the data.

        Parameters
        ----------
        inc: int
          increment of interest for DREAM3D processing.
        """
        regridding_index = self.get_correct_regridding_index(inc)
        geom = GeomGrid.load(
            f"{self.geom_file}_regridded_increment_{self.regridded_inc_list[regridding_index]}.vti"
        )

        cells_rg = geom.cells
        size_rg = geom.size

        dx = np.min(size_rg / cells_rg)

        new_cells = np.rint(size_rg / dx).astype(int)
        gridCoords_cell_0 = damask.grid_filters.coordinates0_point(
            cells_rg, size_rg
        ).reshape((-1, 3), order="F")
        gridCoords_cell_rg = damask.grid_filters.coordinates0_point(
            new_cells, size_rg
        ).reshape((-1, 3), order="F")

        tree = cKDTree(gridCoords_cell_0, boxsize=size_rg)
        map_remesh = tree.query(gridCoords_cell_rg)[1].flatten()
        return map_remesh

    def DAMASKtoDREAM3D_deformed(self, inc):
        """
        Creates a dream3D file from DAMASK output.
        Here the grid deformation is taken into account through regridding.

        Parameters:
        -----------
        inc: int
          increment of interest for DREAM3D processing.
        """
        os.chdir(self.simulation_folder)
        # --------------------------------------------------------------------------
        # Build array of euler angles for each cell
        # --------------------------------------------------------------------------
        d = damask.Result(self.job_file)
        inc_data = d.view(
            increments=inc
        )  # selecting only relevant data to reduce overload

        map_file = h5py.File("regridding.hdf5", "r")
        regridding_steps = [int(i) for i in map_file["phase"].keys()]
        regridding_steps.sort()

        self.get_regridded_inc_list()
        print(self.regridded_inc_list)
        regridding_index = self.get_correct_regridding_index(inc)
        print(self.get_correct_regridding_index)
        print(regridding_index)

        f = h5py.File(self.job_file, "r")
        geom = GeomGrid.load(
            f"{self.geom_file}_regridded_increment_{self.regridded_inc_list[regridding_index]}.vti"
        )
        cells_rg = geom.cells
        size_rg = geom.size

        # regrid the geometry (but at the same dimensions as before)
        map_0to_rg = np.array(map_file[f"map/{regridding_steps[regridding_index]}"])

        # original orientation array
        O_dict = inc_data.get("O")

        # construct the original cell orientation array and then regrid it
        cell_orientation_array = np.zeros((np.prod(cells_rg), 3))

        # construct the original phase array and then regrid it
        phase_ID_array = np.zeros(
            (np.prod(cells_rg)), dtype=np.int32
        )  # need to reshape it later

        # based on the regridding step, find the correct map array with phases
        if regridding_index == 0:
            map_0toRg_phaseBased = f["cell_to/phase"]  # map before the regridding
            map_0toRg_phaseBased = map_0toRg_phaseBased[:, 0][map_0to_rg]
        else:
            map_0toRg_phaseBased = map_file[
                f"phase/{regridding_steps[regridding_index - 1]}"
            ]  # map corresponding to a step before
            map_0toRg_phaseBased_reshaped = np.array(map_0toRg_phaseBased).reshape(
                (len(map_0toRg_phaseBased), 1)
            )
            map_0toRg_phaseBased = map_0toRg_phaseBased_reshaped[:, 0][map_0to_rg]

        # create the original arrays
        for count, p in enumerate(d.phases):
            phase_index = map_0toRg_phaseBased[
                map_0toRg_phaseBased["label"] == f"{p}".encode()
            ]["entry"]
            map_index = np.where(map_0toRg_phaseBased["label"] == f"{p}".encode())[0]
            if len(d.phases) > 1:
                cell_orientation_array[map_index, :] = Rotation(
                    O_dict[p]
                ).as_Euler_angles()[phase_index]
            else:
                cell_orientation_array[map_index, :] = Rotation(
                    O_dict
                ).as_Euler_angles()[phase_index]
            phase_ID_array[map_index] = count + 1

        # --------------------------------------------------------------------------
        job_file_no_ext = os.path.splitext(self.job_file)[0]
        o = h5py.File(f"{job_file_no_ext}_regridded_increment{inc}.dream3D", "w")
        o.attrs["DADF5toDREAM3D"] = "1.0"
        o.attrs["FileVersion"] = "7.0"

        for g in ["DataContainerBundles", "Pipeline"]:  # empty groups (needed)
            o.create_group(g)

        data_container_label = "DataContainers/SyntheticVolumeDataContainer"
        cell_data_label = data_container_label + "/CellData"

        # Data phases
        o[cell_data_label + "/Phases"] = np.reshape(
            phase_ID_array, tuple(np.flip(cells_rg)) + (1,)
        )

        # Data eulers
        orientation_data = cell_orientation_array.astype(np.float32)
        o[cell_data_label + "/Eulers"] = orientation_data.reshape(
            tuple(np.flip(cells_rg)) + (3,)
        )

        # Attributes to CellData group
        o[cell_data_label].attrs["AttributeMatrixType"] = np.array([3], np.uint32)
        o[cell_data_label].attrs["TupleDimensions"] = np.array(cells_rg, np.uint64)

        # Common Attributes for groups in CellData
        for group in ["/Phases", "/Eulers"]:
            o[cell_data_label + group].attrs["DataArrayVersion"] = np.array(
                [2], np.int32
            )
            o[cell_data_label + group].attrs["Tuple Axis Dimensions"] = (
                "x={},y={},z={}".format(*np.array(cells_rg))
            )

        # phase attributes
        o[cell_data_label + "/Phases"].attrs["ComponentDimensions"] = np.array(
            [1], np.uint64
        )
        o[cell_data_label + "/Phases"].attrs["ObjectType"] = "DataArray<int32_t>"
        o[cell_data_label + "/Phases"].attrs["TupleDimensions"] = np.array(
            cells_rg, np.uint64
        )

        # Eulers attributes
        o[cell_data_label + "/Eulers"].attrs["ComponentDimensions"] = np.array(
            [3], np.uint64
        )
        o[cell_data_label + "/Eulers"].attrs["ObjectType"] = "DataArray<float>"
        o[cell_data_label + "/Eulers"].attrs["TupleDimensions"] = np.array(
            cells_rg, np.uint64
        )

        # Create EnsembleAttributeMatrix
        ensemble_label = data_container_label + "/CellEnsembleData"

        # Data CrystalStructures
        # o[ensemble_label + '/CrystalStructures'] = np.uint32(np.array([999,1]))
        o[ensemble_label + "/CrystalStructures"] = np.uint32(
            np.array([999] + [1] * len(d.phases))
        )
        # assuming only cubic crystal structures
        # Damask can give the crystal structure info but need to look into dream3d which crystal structure corresponds to which number
        o[ensemble_label + "/PhaseTypes"] = np.uint32(
            np.array([999] + [Phase_types["Primary"]] * len(d.phases))
        ).reshape((len(d.phases) + 1, 1))
        # also assuming Primary phases
        # there can be precipitates etc as well

        # Attributes Ensemble Matrix
        o[ensemble_label].attrs["AttributeMatrixType"] = np.array([11], np.uint32)
        o[ensemble_label].attrs["TupleDimensions"] = np.array(
            [len(d.phases) + 1], np.uint64
        )

        # Attributes for data in Ensemble matrix
        for group in [
            "CrystalStructures",
            "PhaseTypes",
        ]:  # 'PhaseName' not required MD: But would be nice to take the phase name mapping
            o[ensemble_label + "/" + group].attrs["ComponentDimensions"] = np.array(
                [1], np.uint64
            )
            o[ensemble_label + "/" + group].attrs["Tuple Axis Dimensions"] = (
                f"x={len(d.phases) + 1}"
            )
            o[ensemble_label + "/" + group].attrs["DataArrayVersion"] = np.array(
                [2], np.int32
            )
            o[ensemble_label + "/" + group].attrs["ObjectType"] = "DataArray<uint32_t>"
            o[ensemble_label + "/" + group].attrs["TupleDimensions"] = np.array(
                [len(d.phases) + 1], np.uint64
            )

        # Create geometry info
        geom_label = data_container_label + "/_SIMPL_GEOMETRY"

        o[geom_label + "/DIMENSIONS"] = np.int64(np.array(cells_rg))
        o[geom_label + "/ORIGIN"] = np.float32(np.zeros(3))
        o[geom_label + "/SPACING"] = np.float32(size_rg / cells_rg)

        o[geom_label].attrs["GeometryName"] = "ImageGeometry"
        o[geom_label].attrs["GeometryTypeName"] = "ImageGeometry"
        o[geom_label].attrs["GeometryType"] = np.array([0], np.uint32)
        o[geom_label].attrs["SpatialDimensionality"] = np.array([3], np.uint32)
        o[geom_label].attrs["UnitDimensionality"] = np.array([3], np.uint32)

    def export_DREAM3D(self, inc, dataset, regridded=True):
        """
        Exports datasets from DAMASK data to Dream3D file.
        This is required by SCORE as it needs grain average data.
        The data can be exported only to an existing Dream3D file.
        Following would be the workflow:
        DAMASKtoDREAM3D -> add_orig_grain_IDs -> export_DREAM3D

        Not yet well tested for multiphase data.
        This is REGRIDDED data, not remeshed data, i.e. data is still
        not regularly spaced.

        Parameters
        ----------
        inc: int
          increment of interest for DREAM3D processing.
        dataset: str
          Name of the dataset to be exported.
        regridded: bool
          Whether a regridded geometry is considered or not.
        """
        os.chdir(self.simulation_folder)

        data = self.regrid_datasets_scalar(inc, f"{dataset}")

        job_file_no_ext = os.path.splitext(self.job_file)[0]
        regridding_index = self.get_correct_regridding_index(inc)
        if regridded:
            o = h5py.File(f"{job_file_no_ext}_regridded_increment{inc}.dream3D", "a")
            geom = GeomGrid.load(
                f"{self.geom_file}_regridded_increment_{self.regridded_inc_list[regridding_index]}.vti"
            )
        else:
            o = h5py.File(f"{job_file_no_ext}_increment{inc}.dream3D", "a")
            geom = GeomGrid.load(f"{self.geom_file}.vti")

        cells_rg = geom.cells
        size_rg = geom.size

        # data containers for dream3D
        data_container_label = "DataContainers/SyntheticVolumeDataContainer"
        cell_data_label = data_container_label + "/CellData"
        # Data rho
        data = data.astype(np.float32)
        o[cell_data_label + f"/{dataset}"] = data.reshape(
            tuple(np.flip(cells_rg)) + (1,)
        )

        # attributes tot_density
        o[cell_data_label + f"/{dataset}"].attrs["DataArrayVersion"] = np.array(
            [2], np.int32
        )
        o[cell_data_label + f"/{dataset}"].attrs["Tuple Axis Dimensions"] = (
            "x={},y={},z={}".format(*np.array(cells_rg))
        )

        o[cell_data_label + f"/{dataset}"].attrs["ComponentDimensions"] = np.array(
            [1], np.uint64
        )
        o[cell_data_label + f"/{dataset}"].attrs["ObjectType"] = "DataArray<float>"
        o[cell_data_label + f"/{dataset}"].attrs["TupleDimensions"] = np.array(
            cells_rg, np.uint64
        )

    def remesh_DREAM3D(self, inc):
        """
        Creates a remeshed DREAM3D file.
        This is required by SCORE as it needs regularly spaced data.
        Pre-requisite is to have an existing dream3D file with the regridded data.
        Following would be the workflow:
        DAMASKtoDREAM3D -> export_DREAM3D -> export_external_data -> remeshed

        Not yet tested for multiphase data.

        Parameters
        ----------
        inc: int
          increment of interest for DREAM3D processing.
        """
        os.chdir(self.simulation_folder)
        d = damask.Result(self.job_file)

        job_file_no_ext = os.path.splitext(self.job_file)[0]
        o = h5py.File(f"{job_file_no_ext}_regridded_increment{inc}.dream3D", "r")
        regridding_index = self.get_correct_regridding_index(inc)
        geom = GeomGrid.load(
            f"{self.geom_file}_regridded_increment_{self.regridded_inc_list[regridding_index]}.vti"
        )

        cells_rg = geom.cells
        size_rg = geom.size

        dx = np.min(size_rg / cells_rg)

        new_cells = np.rint(size_rg / dx).astype(int)

        map_remesh = self.get_remesh_map(inc)

        w = h5py.File(f"{job_file_no_ext}_remeshed_increment{inc}.dream3D", "w")
        w.attrs["DADF5toDREAM3D"] = "1.0"
        w.attrs["FileVersion"] = "7.0"

        for g in ["DataContainerBundles", "Pipeline"]:  # empty groups (needed)
            w.create_group(g)

        # data containers for dream3D
        data_container_label = "DataContainers/SyntheticVolumeDataContainer"
        cell_data_label = data_container_label + "/CellData"

        # read and remesh data one by one
        # Eulers
        data = np.array(o[cell_data_label + f"/Eulers"]).reshape(np.prod(cells_rg), 3)
        remeshed_data = data[map_remesh]

        w[cell_data_label + "/Eulers"] = remeshed_data.reshape(
            tuple(np.flip(new_cells)) + (3,)
        )

        # phases
        data = np.array(o[cell_data_label + f"/Phases"]).reshape(np.prod(cells_rg), 1)
        remeshed_data = data[map_remesh]

        w[cell_data_label + "/Phases"] = remeshed_data.reshape(
            tuple(np.flip(new_cells)) + (1,)
        )

        # Data rho
        data = np.array(o[cell_data_label + f"/tot_density"]).reshape(
            np.prod(cells_rg), 1
        )
        remeshed_data = data[map_remesh]

        w[cell_data_label + "/tot_density"] = remeshed_data.reshape(
            tuple(np.flip(new_cells)) + (1,)
        )

        # Attributes to CellData group
        w[cell_data_label].attrs["AttributeMatrixType"] = np.array([3], np.uint32)
        w[cell_data_label].attrs["TupleDimensions"] = np.array(new_cells, np.uint64)

        # Common Attributes for groups in CellData
        for group in ["/Phases", "/Eulers", "/tot_density"]:
            w[cell_data_label + group].attrs["DataArrayVersion"] = np.array(
                [2], np.int32
            )
            w[cell_data_label + group].attrs["Tuple Axis Dimensions"] = (
                "x={},y={},z={}".format(*np.array(new_cells))
            )

        # phase attributes
        w[cell_data_label + "/Phases"].attrs["ComponentDimensions"] = np.array(
            [1], np.uint64
        )
        w[cell_data_label + "/Phases"].attrs["ObjectType"] = "DataArray<int32_t>"
        w[cell_data_label + "/Phases"].attrs["TupleDimensions"] = np.array(
            new_cells, np.uint64
        )

        # Eulers attributes
        w[cell_data_label + "/Eulers"].attrs["ComponentDimensions"] = np.array(
            [3], np.uint64
        )
        w[cell_data_label + "/Eulers"].attrs["ObjectType"] = "DataArray<float>"
        w[cell_data_label + "/Eulers"].attrs["TupleDimensions"] = np.array(
            new_cells, np.uint64
        )

        # attributes tot_density
        w[cell_data_label + f"/tot_density"].attrs["ComponentDimensions"] = np.array(
            [1], np.uint64
        )
        w[cell_data_label + f"/tot_density"].attrs["ObjectType"] = "DataArray<float>"
        w[cell_data_label + f"/tot_density"].attrs["TupleDimensions"] = np.array(
            new_cells, np.uint64
        )

        # Create EnsembleAttributeMatrix
        ensemble_label = data_container_label + "/CellEnsembleData"

        # Data CrystalStructures
        # o[ensemble_label + '/CrystalStructures'] = np.uint32(np.array([999,1]))
        w[ensemble_label + "/CrystalStructures"] = np.uint32(
            np.array([999] + [1] * len(d.phases))
        )
        # currently doing this only for single phase materials
        # assuming only cubic crystal structures
        w[ensemble_label + "/PhaseTypes"] = np.uint32(
            np.array([999] + [Phase_types["Primary"]] * len(d.phases))
        ).reshape((len(d.phases) + 1, 1))
        # w[ensemble_label + '/PhaseTypes']        = np.uint32(np.array([999] + [Phase_types['Primary']]*1)).reshape((2,1))
        # Attributes Ensemble Matrix
        w[ensemble_label].attrs["AttributeMatrixType"] = np.array([11], np.uint32)
        w[ensemble_label].attrs["TupleDimensions"] = np.array(
            [len(d.phases) + 1], np.uint64
        )

        # Attributes for data in Ensemble matrix
        for group in [
            "CrystalStructures",
            "PhaseTypes",
        ]:  # 'PhaseName' not required MD: But would be nice to take the phase name mapping
            w[ensemble_label + "/" + group].attrs["ComponentDimensions"] = np.array(
                [1], np.uint64
            )
            w[ensemble_label + "/" + group].attrs["Tuple Axis Dimensions"] = (
                f"x={len(d.phases) + 1}"
            )
            w[ensemble_label + "/" + group].attrs["DataArrayVersion"] = np.array(
                [2], np.int32
            )
            w[ensemble_label + "/" + group].attrs["ObjectType"] = "DataArray<uint32_t>"
            w[ensemble_label + "/" + group].attrs["TupleDimensions"] = np.array(
                [len(d.phases) + 1], np.uint64
            )
        # Create geometry info
        geom_label = data_container_label + "/_SIMPL_GEOMETRY"

        w[geom_label + "/DIMENSIONS"] = np.int64(np.array(new_cells))
        w[geom_label + "/ORIGIN"] = np.float32(np.zeros(3))
        w[geom_label + "/SPACING"] = np.float32([dx] * 3)

        w[geom_label].attrs["GeometryName"] = "ImageGeometry"
        w[geom_label].attrs["GeometryTypeName"] = "ImageGeometry"
        w[geom_label].attrs["GeometryType"] = np.array([0], np.uint32)
        w[geom_label].attrs["SpatialDimensionality"] = np.array([3], np.uint32)
        w[geom_label].attrs["UnitDimensionality"] = np.array([3], np.uint32)

    def add_nucleation_max_rho(self, inc, max_dislocation_threshold):
        """
        Detects nuclei for hotspots with high dislocation density.
        The nucleation flags are added as a dataset to the Dream3D file.
        The data can be exported only to an existing Dream3D file.
        Following would be the workflow:
        DAMASKtoDREAM3D_deformed -> export_DREAM3D -> remesh_DREAM3D

        Not yet tested for multiphase data.

        Parameters
        ----------
        inc: int
          increment of interest for DREAM3D processing.
        max_dislocation_threshold: float
          Threshold used for nucleation criterion.
        """
        os.chdir(self.simulation_folder)

        data_container_label = "DataContainers/SyntheticVolumeDataContainer"
        cell_data_label = data_container_label + "/CellData"

        job_file_no_ext = os.path.splitext(self.job_file)[0]

        o = h5py.File(f"{job_file_no_ext}_remeshed_increment{inc}.dream3D", "a")
        cells_rg = np.array(
            o["DataContainers/SyntheticVolumeDataContainer/_SIMPL_GEOMETRY/DIMENSIONS"]
        )
        size_rg = (
            np.array(
                o["DataContainers/SyntheticVolumeDataContainer/_SIMPL_GEOMETRY/SPACING"]
            )
            * cells_rg
        )
        data = np.array(
            o["DataContainers/SyntheticVolumeDataContainer/CellData/tot_density"]
        ).reshape(np.prod(cells_rg))
        phase_array = np.array(
            o["DataContainers/SyntheticVolumeDataContainer/CellData/Phases"]
        ).reshape(np.prod(cells_rg))

        # nuclei detection
        gridCoords_point_initial_matrix = damask.grid_filters.coordinates0_point(
            cells_rg, size_rg
        )
        gridCoords_point_initial = gridCoords_point_initial_matrix.reshape(
            (-1, gridCoords_point_initial_matrix.shape[-1]), order="F"
        )
        tree = cKDTree(gridCoords_point_initial, boxsize=size_rg)

        nuc_flag = np.zeros((len(gridCoords_point_initial)), dtype=np.int32)

        nuc_indices = np.where(data > max_dislocation_threshold)

        nuc_flag[nuc_indices] = 1
        print("Added nuclei:", len(np.where(nuc_flag != 0)[0]))

        # Data nucleation
        o[cell_data_label + "/nuc_flag"] = nuc_flag.reshape(
            tuple(np.flip(cells_rg)) + (1,)
        )

        # attributes tot_density
        o[cell_data_label + "/nuc_flag"].attrs["DataArrayVersion"] = np.array(
            [2], np.int32
        )
        o[cell_data_label + "/nuc_flag"].attrs["Tuple Axis Dimensions"] = (
            "x={},y={},z={}".format(*np.array(cells_rg))
        )

        o[cell_data_label + "/nuc_flag"].attrs["ComponentDimensions"] = np.array(
            [1], np.uint64
        )
        o[cell_data_label + "/nuc_flag"].attrs["ObjectType"] = "DataArray<int32_t>"
        o[cell_data_label + "/nuc_flag"].attrs["TupleDimensions"] = np.array(
            cells_rg, np.uint64
        )
