# SimGrid Template Project using S4U and cmake

This project contains all the cmake configuration that you need to
start your own project using the S4U interface of SimGrid, along with
several small example applications.

## How to proceed
1. Fork this repository. 
1. Remove the fork relationship. Go to **Settings (⚙)** >
   **Edit Project** and click the **"Remove fork relationship"** button.
1. Rename the repository in GitLab to match the name you want.
1. Change the project name on top of `CMakeLists.txt`.
1. (optional) copy cpp code of another S4U example, that correspond to
   what you want to build.
1. Start editing the source code. If you add more files, list them in
   the `add_executable` line of your `CMakeLists.txt` file.
   
## Dependencies
- SimGrid version 3.22 or better is required
- cmake, g++ or clang++, libboost-dev
