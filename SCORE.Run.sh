#!/bin/sh

cd examples/verification
mpirun -n 1 ./../../build/score 0 SCORE.Config.SimID.0.nxs

# cd examples/damask
# mpirun -n 1 ./../../build/score 1 SCORE.Config.SimID.1.nxs
cd ../../
