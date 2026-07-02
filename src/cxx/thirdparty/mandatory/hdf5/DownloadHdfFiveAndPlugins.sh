#!/usr/bin/env bash

set -euo pipefail

wget https://github.com/HDFGroup/hdf5/archive/refs/tags/hdf5_1.14.6.tar.gz

wget https://github.com/HDFGroup/hdf5_plugins/releases/download/hdf5-1.14.6/hdf5_plugins-1.14.tar.gz
wget https://github.com/madler/zlib/releases/download/v1.3.1/zlib-1.3.1.tar.gz
wget https://github.com/zlib-ng/zlib-ng/archive/refs/tags/2.2.2.tar.gz
wget https://github.com/MathisRosenhauer/libaec/releases/download/v1.1.3/libaec-1.1.3.tar.gz

mkdir -p "build"
mv "hdf5_1.14.6.tar.gz" "hdf5-1.14.6.tar.gz"
tar -xvf "hdf5-1.14.6.tar.gz"
mv "hdf5-hdf5_1.14.6" "hdf5-1.14.6"
