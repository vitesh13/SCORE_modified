Use the CmakeLists in the main folder for compiling
Copy it to the build folder
cd to the build folder
Then use the following command

cmake -DCMAKE_BUILD_TYPE=Release

Before running the code, you might have to make some changes in the source code. 
Assuming that you downloaded SCORE to some folder, lets name it:
source_folder = "/home/SCORE_orig"

The source folder after download should have these sub-folders:
src, example_files.
src has all the source code and example_files are the files that you would need to run the code on your computer. 
In the source code, the file paths have been changed for working on any general computer. 

mpirun -n 1 ./build/score_test 0 example_files/SCORE.Debug.MartinDiehl.64.SimID.6403.uds 

