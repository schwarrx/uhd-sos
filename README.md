# uhd-sos
Code, designs, and documentation for work on UHD-SoS  

UHD-SoS stands for Ultra High Definition Structures-of-Structures. The aim of this project is to perform manufacturability analysis for UHD-SoS designs. The project requires the following dependencies

## Dependencies

Currently, our build process depends on the following packages:
- [CMake](https://cmake.org/) 3.4+
- [Boost](http://www.boost.org/) 1.61+
- [CUDA](https://developer.nvidia.com/cuda-downloads) 7.5+
- [ArrayFire](https://github.com/arrayfire) with CUDA bindings

To install the dependencies on macOS, run
```sh
brew install cmake boost 
```

On Ubuntu 16.10 (yakkety), run
```sh
sudo apt-get install -y build-essential cmake zlib1g-dev  libboost-dev
```
