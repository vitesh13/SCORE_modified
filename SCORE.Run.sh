#!/bin/sh

cd examples/verification
mpirun -n 1 ./../../build/score 0 SCORE.Config.SimID.0.nxs
cd ../../

