# Introduction 
This is a simulation of a data-sharing database system with a global lock manager. 
It is an early-stage research project. 

# Getting Started
The project uses [CMake](https://cmake.org/) and Boost (only tested on Boost 1.80).

## Building from the CLI
1. Create a build directory. At the root of the project run: ``` mkdir build ```.
2. Create build/debug and build/release directory for debug builds and release builds, respectively. At the root of the project run: ``` mkdir build/debug && mkdir build/release ```.
3. To prepare for a debug build at the root run: ``` cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug ```.   
   Similarly, for a release build, at the root run: ``` cmake -S . -B build -DCMAKE_BUILD_TYPE=Release ```.
4. Build by running ``` cmake --build build ```. Use ```-j c``` to use c cores when building.
5. run test with ``` ctest ``` after changing to the appropriate build directory ``` cd  build/debug ```.

# Project Structure
The project is made of the following modules:   
1. **//src/common**: contains configs, type definitions, core data structures (list, map, queue) common to all other modules.
2. **//src/storage**: contains a Storage API under which we implement different GLM protocols. The API provides access to different pages and records.    
    &emsp;**/buffer_manager**: contains the buffer manager implementation that uses the clock algorithm.     
    &emsp;**/lock_manager**: contains the lock manager implementation which functions as a GLM & LLM on one compute node and only an LLM on the other compute node.    
    &emsp;**/log_manager**: contains the log manager implementation which simulates an append to a log.     
3. **//src/processor**: is a task system that has multiple worker threads. A query is pushed to a common queue from which all threads pop to execute transactions.      
4. **//main**: contains the client and compute node main functions.

# References
The YCSB and synthetic data generators under `include/benchmark/ycsb` follow:
- Brian F. Cooper et al. [Benchmarking Cloud Serving Systems with YCSB](https://doi.org/10.1145/1807128.1807152). SoCC 2010.
- Jim Gray et al. [Quickly Generating Billion-Record Synthetic Databases](https://doi.org/10.1145/191839.191886). SIGMOD 1994.

# Contributing

This project welcomes contributions and suggestions.  Most contributions require you to agree to a
Contributor License Agreement (CLA) declaring that you have the right to, and actually do, grant us
the rights to use your contribution. For details, visit https://cla.opensource.microsoft.com.

When you submit a pull request, a CLA bot will automatically determine whether you need to provide
a CLA and decorate the PR appropriately (e.g., status check, comment). Simply follow the instructions
provided by the bot. You will only need to do this once across all repos using our CLA.

This project has adopted the [Microsoft Open Source Code of Conduct](https://opensource.microsoft.com/codeofconduct/).
For more information see the [Code of Conduct FAQ](https://opensource.microsoft.com/codeofconduct/faq/) or
contact [opencode@microsoft.com](mailto:opencode@microsoft.com) with any additional questions or comments.

# Trademarks

This project may contain trademarks or logos for projects, products, or services. Authorized use of Microsoft 
trademarks or logos is subject to and must follow 
[Microsoft's Trademark & Brand Guidelines](https://www.microsoft.com/en-us/legal/intellectualproperty/trademarks/usage/general).
Use of Microsoft trademarks or logos in modified versions of this project must not cause confusion or imply Microsoft sponsorship.
Any use of third-party trademarks or logos are subject to those third-party's policies.
