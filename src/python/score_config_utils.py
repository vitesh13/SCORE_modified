# SPDX-License-Identifier: GNU General Public License v3.0 or later

import hashlib

import numpy as np
from pint import UnitRegistry

ureg = UnitRegistry()
DEGREE_TO_RADIAN = np.pi / 180
# ureg.formatter.default_format = "D"
# https://pint.readthedocs.io/en/stable/user/formatting.html


def recursive_assign(path, idx, leaf_idx, obj, nested_dict):
    # print(f"{path}, {leaf_idx}, {nested_dict}")
    if -1 < idx < leaf_idx:
        if path[idx] not in nested_dict:
            nested_dict[path[idx]] = {}
        recursive_assign(path, idx + 1, leaf_idx, obj, nested_dict[path[idx]])
    else:
        if path[idx] not in nested_dict:
            if isinstance(obj, ureg.Quantity):
                if path[idx] not in nested_dict:
                    nested_dict[path[idx]] = {}
                    if obj.dimensionless:
                        if np.shape(obj.magnitude) == ():
                            nested_dict[path[idx]]["value"] = (
                                obj.magnitude.item()
                            )  # ["value"] value only for consistency
                        else:
                            nested_dict[path[idx]]["value"] = obj.magnitude.tolist()
                        if obj.units == "radian":
                            nested_dict[path[idx]]["@units"] = f"{obj.units}"
                    else:
                        if np.shape(obj.magnitude) == ():
                            nested_dict[path[idx]]["value"] = (
                                obj.magnitude.item()
                            )  # ["value"] value only for consistency
                        else:
                            nested_dict[path[idx]]["value"] = obj.magnitude.tolist()
                        nested_dict[path[idx]]["@units"] = f"{obj.units}"
            elif isinstance(obj, (str, bool)):
                if path[idx] not in nested_dict:
                    # if path[idx] == "@version":
                    #     print(f"{path} >>>> {path[idx]} >>>> {obj} >>>> {nested_dict}")
                    # nested_dict[path[idx]] = {}
                    # nested_dict[path[idx]]["value"] = obj  # ["value"] value only for consistency
                    nested_dict[path[idx]] = obj  # ["value"] value only for consistency
            elif isinstance(obj, list):
                if all(True if isinstance(val, str) else False for val in obj):
                    nested_dict[path[idx]] = obj  # ["value"] value only for consistency
    return nested_dict


def get_sha256_of_file_content(fpath) -> str:
    """Compute a SHA256 hashvalue of given file."""
    fh = hashlib.sha256()
    try:
        with open(fpath, "rb") as fp:
            while True:
                chunk = fp.read(fh.block_size)
                if not chunk:
                    break
                fh.update(chunk)
    except FileNotFoundError:
        return f"{fh.hexdigest()}"
    return f"{fh.hexdigest()}"


def get_nexus_version() -> str:
    """Compute git describe version of the NeXus definitions used."""
    try:
        with open("../cxx/thirdparty/mandatory/nexus-version.txt") as fp:
            return f"{fp.read()}"
    except FileNotFoundError:
        return "0"

def get_file_size(file_name):
    print(f"{np.around(os.path.getsize(file_name) / 1024 / 1024, decimals=3)} MiB")
