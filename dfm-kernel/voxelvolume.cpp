/*
 * voxelvolume.cpp
 *
 *  Created on: April 13, 2017
 *      Author: nelaturi
 */
 
#include "voxelvolume.hpp"


using namespace std;

voxelVolume::voxelVolume(std::vector<byte> data, int dim) {
	/** Construct an empty voxel volume with given data 
	* and dimensions
	*/ 
	hostVol = data;
	dims[0] = dim; 
	dims[1] = dim;
	dims[2] = dim;
	 
}

voxelVolume::voxelVolume(int dim) {
	/** Construct an empty voxel volume with given dimensions
	*/  
	const int size = pow(dim,3);
	//hostVol = new byte[size];
	hostVol = std::vector<byte>(size, 0); 
	dims[0] = dim; 
	dims[1] = dim;
	dims[2] = dim;
	 
}

void voxelVolume(af::array x) {
	/** Construct a voxel volume from an arrayfire array
	*/
	x = x.as(f32);
	std::cout << "Create af vol " << std::endl;
	//// FIX THIS = iterate over array
	
}

voxelVolume::voxelVolume(std::string filespec)
{
	/** Construct a voxel volume from a binvox file
	*/
	std::cout << "Reading binvox file " << std::endl;
	// reads a binvox file 
	static int size;
	   

	std::ifstream *input = new std::ifstream(filespec.c_str(), ios::in | ios::binary);
	processHeader(input);
 
	size = dims[0] * dims[1] * dims[2]; 
	//hostVol = new byte[size];
	hostVol = std::vector<byte>(size, 0); 
	
	// read voxel data 
	byte value;
	byte count;
	int index = 0;
	int end_index = 0;
	int nr_voxels = 0;

	input->unsetf(ios::skipws);  // need to read every byte now (!)
	*input >> value;  // read the linefeed char

	while((end_index < size) && input->good()) {
		*input >> value >> count;

		if (input->good()) {
			end_index = index + count;
			if (end_index > size) throw std::exception();
			for(int i=index; i < end_index; i++)
				 hostVol[i] =value; 
			if (value) nr_voxels += count;
			index = end_index;
		}  // if file still ok
	}  // while

	input->close();

	// SN - code below copies the voxels into an arrayfire array
	//af::array A = af::array(width, depth, height, voxels);
	//A = A.as(f32);
	
	/*
	std::cout << "read " << nr_voxels << " voxels" << std::endl; 
  	for (int i = 0; i < nr_voxels; i++){
  		unsigned int x,y,z;
  		index2xyz(i,x,y,z);
  		if (hostVol[i] != 0.0){
  		std::cout << "1-d index =" << i <<  " has value " << (float) hostVol[i] 
  		<< " and 3d index = [ " << x << "," << y << "," << z << "]" << std::endl;
  		}
  	} 
  	*/

}

void voxelVolume::processHeader(std::ifstream* input){
	/// Process the header for a binvox file std::ifstream and set the dimensions
	static int depth, height, width;
	static float tx, ty, tz; // these are not really used but kept for completeness
	static float scale; // as above.
	static int version; // ditto
	
	string line;
	*input >> line;  // #binvox 
	if (line.compare("#binvox") != 0) {
		std::cout << "Error: first line reads [" << line << "] instead of [#binvox]" << std::endl;
		delete input; 
		throw std::exception();
	} 
	*input >> version; 
	depth = -1;
	bool done = false;
	
	while(input->good() && !done) {
		*input >> line;
		if (line.compare("data") == 0) done = true;
		else if (line.compare("dim") == 0) {
			*input >> depth >> height >> width;
			dims[0] = width; 
			dims[1] = height;
			dims[2] = depth;
		}
		else if (line.compare("translate") == 0 ) {
			*input >> tx >> ty >> tz;
		} 
		else if (line.compare("scale") == 0) {
			*input >> scale;
		}
		else {
			std::cout << "  unrecognized keyword [" << line << "], skipping" << std::endl;
			char c;
			do {  // skip until end of line
				c = input->get();
			} while(input->good() && (c != '\n'));
		}
	}
	
	if (!done) {
		std::cout << "  error reading header" << std::endl;
		throw std::exception();
	}
	if (depth == -1) {
		std::cout << "  missing dimensions in header" << std::endl;
		throw std::exception();
	}
	
}






void voxelVolume::visualizeVolume(float levelSet, float decimation){
	/* Use Marching Cubes and Mesh decimation to visualize the 
	* voxel volume generated on the host. 
	*/
 
	visualizeRenderWindow(extractLevelSetAndSimplify
		(create3dVTKImage(hostVol.data(), getDims()), levelSet, decimation));

}



