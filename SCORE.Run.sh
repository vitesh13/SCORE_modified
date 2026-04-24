#!/bin/sh

# if desired set the maximum number of OpenMP processes
# if not set by default all available cores will be used
# export OMP_NUM_THREADS=1

mode="$1"

if [ "$mode" != "0" ] && [ "$mode" != "1" ]; then
  echo "Warning: invalid mode '$mode' (expected 0=verification, 1=damask)" >&2
fi

if [ "$mode" = "0" ]; then
  run_type="verification"
elif [ "$mode" = "1" ]; then
  run_type="damask"
else
  run_type="unknown"
fi

echo "Running: $run_type"

cd examples/verification
mpirun -n 1 ./../../build/score $mode SCORE.Config.SimID.$mode.nxs 1>SCORE.Results.SimID.$mode.nxs.STDOUT.txt 2>SCORE.Results.SimID.$mode.nxs.STDERR.txt
cd ../../

cd examples/damask
mpirun -n 1 ./../../build/score $mode SCORE.Config.SimID.$mode.nxs 1>SCORE.Results.SimID.$mode.nxs.STDOUT.txt 2>SCORE.Results.SimID.$mode.nxs.STDERR.txt
cd ../../
