/*
 * voxelvolume.hpp
 *
 *  Created on: April 13, 2017
 *      Author: nelaturi
 */

#ifndef VOXVOLUME_HPP_
#define VOXVOLUME_HPP_

#include <fstream>
#include <iostream>
#include <stdlib.h> 
#include <arrayfire.h>
#include "graphics.hpp"

typedef unsigned char byte;

class voxelVolume {
    /** This class creates a voxel volume from a binvox file on the host
     * and copies it to the requested device to return an arrayfire array.
     * (assuming the device can hold the array). The class also provides
     * methods to create 'chunks' of voxel volumes that can in turn be
     * processed on multiple GPUs
     */

protected:
    //byte* hostVol; // volume stored as a single contiguous block of memory on the host
    std::vector<byte> hostVol;
    int dims[3]; // volume dimensions (i.e. voxels in each dimension)
    void processHeader(std::ifstream* input); // process binvox header

public:

    voxelVolume(std::string filespec);
    //voxelVolume(af::array);
    voxelVolume(int dim);
    voxelVolume(std::vector<byte>, int dim);

    int* getDims() {
        return (dims);
    }

    std::vector<byte> getHostVolume() {
        return (hostVol);
    }

    int xyzToIndex(int x, int y, int z, int dim) {
        // return the linear index corresponding to a 3d index
        return (z * dim * dim + y * dim + x);
    }

    int* indexToxyz(int index, int dim) {
        /// return the 3d indices corresponding to a 1-d index
        int* indices = new int[3];
        int ij = dim * dim;
        indices[0] = (index % ij) % dim;
        indices[1] = (index % ij) / dim;
        indices[2] = index / ij;
        return (indices);

    }

    void visualizeVolume(float levelSet, float decimation);

    ~voxelVolume() {
    }
    ;
    // destructor

};

#endif /* VOXVOLUME_HPP_ */
