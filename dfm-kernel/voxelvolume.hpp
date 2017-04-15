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

// vtk  
#include <vtkSmartPointer.h>
#include <vtkImageData.h> 

 
typedef unsigned char byte;  
 
class voxelVolume{
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
	
	int* getDims(){
		return dims;
	}
	
	std::vector<byte> getHostVolume(){
		return hostVol;
	}
	
	
	void index2xyz(unsigned int index, 
                   unsigned int &x, 
                   unsigned int &y, 
                   unsigned int &z) const {
        /// Convert a 1-d index to 3-d x,y,z coordinates
        unsigned rc = dims[0]*dims[1];
        z = index/rc;
        y = (index % rc) / dims[1];
        x = (index % rc) % dims[1];
    }
	
	vtkSmartPointer<vtkImageData> create3dVTKImage();
	 
	void visualizeVolume( float levelSet, float decimation );
	
	~voxelVolume(){ 
	}; // destructor
	
	


};
  
#endif /* VOXVOLUME_HPP_ */
