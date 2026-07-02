@echo off
echo Current Directory: %CD%
echo CMake Version:
cmake --version
echo Running CMake...
cmake -S . -B build -DCMAKE_BUILD_TYPE=RelWithDebInfo
echo CMake Finished.
pause