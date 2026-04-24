# SPDX-License-Identifier: GNU General Public License v3.0 or later

import datetime as dt
import os
from typing import Any

import flatdict as fd
import h5py
import numpy as np
import yaml
from score_config_utils import recursive_assign, ureg


class ConfigBase:
    def __init__(self):
        self.cfg: dict[str, Any] = {}

    def get_config(self):
        for key, qnt in sorted(self.cfg.items()):
            print(f"{key}, {qnt}")

    def write_config_yaml_file(self):
        """Write YAML config file to be used in SCORE."""
        template = {}
        for key, obj in self.cfg.items():
            path = key.split("/")
            recursive_assign(path, 0, len(path) - 1, obj, template)

        with open(self.yaml_file_name, "w") as fp:
            yaml.dump(template, fp, default_flow_style=None, sort_keys=False)
        print(f"{self.yaml_file_name} written")

    def get_flat_list_of_hfive_concept_paths(self):
        """Write HDF5 file to be used in NOMAD."""
        if os.path.isfile(self.yaml_file_name):
            with open(self.yaml_file_name, encoding="utf-8") as stream:
                for key, val in fd.FlatDict(
                    yaml.safe_load(stream), delimiter="/"
                ).items():
                    # print(f"{key}, {val}")
                    if key.endswith("value"):
                        print(f"/{key[0:-6]}: {val}")
                    else:
                        print(f"/{key}: {val}")

    def write_config_hfive_file(self):
        with h5py.File(self.hdf5_file_name, "w") as h5w:
            for key, obj in self.cfg.items():
                if key.endswith("@NX_class"):  # "@default", "@version")):
                    hpath = f"/{key[0 : key.rfind('@')]}"
                    attr_name = f"{key[key.rfind('@') + 1 :]}"
                    # print(f"group >>>> {hpath}, {attr_name}")
                    if hpath != "/":
                        grpnm = hpath[0:-1]
                        if grpnm not in h5w:
                            grp = h5w.create_group(grpnm)
                            grp.attrs[attr_name] = obj
                        else:
                            h5w[grpnm].attrs[attr_name] = obj
                    else:
                        h5w.attrs[attr_name] = obj
            print("Attributes of groups created")
            # instantiate fields which is possible because groups now exist and
            # the h5py library internally assures that the data end up written
            # in that group mentioned because making e.g. create_dataset calls
            # can use even deeply nested paths on empty hdf5 and will recursively
            # add all those missing groups along the path from the leaf upwards to root
            for key, obj in self.cfg.items():
                if "@" not in key:  # not key.endswith("@NX_class"):
                    hpath = f"/{key}"
                    # print(f"field >>>> {key}, {hpath}, {type(obj)}")
                    if isinstance(obj, ureg.Quantity):
                        if obj.dimensionless:
                            if np.shape(obj.magnitude) == ():
                                dst = h5w.create_dataset(hpath, data=obj.magnitude)
                            else:
                                dst = h5w.create_dataset(hpath, data=obj.magnitude)
                            if obj.units == "radian":
                                dst.attrs["units"] = f"{obj.units}"
                        else:
                            if np.shape(obj.magnitude) == ():
                                dst = h5w.create_dataset(hpath, data=obj.magnitude)
                            else:
                                dst = h5w.create_dataset(hpath, data=obj.magnitude)
                            dst.attrs["units"] = f"{obj.units}"
                    elif isinstance(obj, (str, bool)):
                        dst = h5w.create_dataset(hpath, data=obj)
                    elif isinstance(obj, list):
                        if all(True if isinstance(val, str) else False for val in obj):
                            dst = h5w.create_dataset(hpath, data=obj)
            print("Fields created")
            # add attributes to existent fields and groups which are not NX_class
            for key, obj in self.cfg.items():
                if ("@" in key) and (not key.endswith("@NX_class")):
                    hpath = f"/{key[0 : key.rfind('@')]}"
                    if hpath != "//":
                        attr_name = f"{key[key.rfind('@') + 1 :]}"
                        # print(f"attribute >>>> {hpath}, {attr_name}")
                        h5w[hpath].attrs[attr_name] = obj
            print("Attributes other than NX_class")
            h5w["/entry1/start_time"] = (
                f"{dt.datetime.now(dt.timezone.utc).isoformat().replace('+00:00', 'Z')}"
            )
            print(f"{self.hdf5_file_name} written")
