/*
 * voxelvolume.hpp
 *
 *  Created on: April 13, 2017
 *      Author: nelaturi
 */
 
#ifndef VOXVOLUME_HPP_
#define VOXVOLUME_HPP_

#include "boost/multi_array.hpp" 
#include <fstream>
#include <iostream>
#include <stdlib.h>
 
using namespace std;
 
typedef unsigned char byte;
typedef boost::multi_array<byte, 1> hostVolume; // specifically meant for a binary voxelization stored on host
//typedef hostArray::extent_range range;
 
class voxelVolume{
	/** This class creates a voxel volume from a binvox file on the host
	* and copies it to the requested device to return an arrayfire array.
	* (assuming the device can hold the array). The class also provides
	* methods to create 'chunks' of voxel volumes that can in turn be 
	* processed on multiple GPUs
	*/
	
private:
	byte* hostVol; // volume stored as a single contiguous block of memory on the host
	int dims[3]; // volume dimensions (i.e. voxels in each dimension)
	void processHeader(ifstream* input); // process binvox header
	
public:
	
	voxelVolume(std::string filespec);
	
	int* getDims(){
		return dims;
	}
	
	~voxelVolume(){
		delete hostVol;
	}; // destructor
	


};
  
#endif /* VOXVOLUME_HPP_ */
