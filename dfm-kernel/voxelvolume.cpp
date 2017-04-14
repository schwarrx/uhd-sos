/*
 * voxelvolume.cpp
 *
 *  Created on: April 13, 2017
 *      Author: nelaturi
 */
 
#include "voxelvolume.hpp"

voxelVolume::voxelVolume(std::string filespec)
{
	cout << "Reading binvox file " << endl;
	// reads a binvox file 
	static int size;
	   

	ifstream *input = new ifstream(filespec.c_str(), ios::in | ios::binary);
	processHeader(input);

	std::cout << "Done processing header " << std::endl;
	size = dims[0] * dims[1] * dims[2]; 
	hostVol = new byte[size];
	
	if (!hostVol) {
		cout << "  error allocating memory" << endl;
		throw std::exception();
	}
	
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
	//cout << "read " << nr_voxels << " voxels" << endl;
	//cout << "Array dimensions = " << A.dims() << endl;
	//af_print(A);
  

}

void voxelVolume::processHeader(ifstream* input){
	/// Process the header for a binvox file ifstream and set the dimensions
	static int depth, height, width;
	static float tx, ty, tz; // these are not really used but kept for completeness
	static float scale; // as above.
	static int version; // ditto
	
	string line;
	*input >> line;  // #binvox 
	if (line.compare("#binvox") != 0) {
		cout << "Error: first line reads [" << line << "] instead of [#binvox]" << endl;
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
			cout << "  unrecognized keyword [" << line << "], skipping" << endl;
			char c;
			do {  // skip until end of line
				c = input->get();
			} while(input->good() && (c != '\n'));
		}
	}
	
	if (!done) {
		cout << "  error reading header" << endl;
		throw std::exception();
	}
	if (depth == -1) {
		cout << "  missing dimensions in header" << endl;
		throw std::exception();
	}
	
}


/*
af::array chunkConvolution(af::array X, af::array Y, bool correlation, int computeDevice, int storageDevice){
	// assuming af::array x is very large and af::array y is small 
	// and fits on the gpu, this function breaks up x into 
	// small chunks and computes the convolution 
	// also assume x is a power of 2 three-dimensional af::array
	
		int res = 256; // we want to break up the input into res^3 chunks
		int dim1d = X.dims()[0];
		int n = dim1d/res;
		int nchunks = pow(n,3);
		cout << " We will divide the input volume into " << nchunks << " chunks" << endl;
		// do this the naive way first
		int chunk = 0;
		 
		af::array result(dim1d, dim1d, dim1d,f32);  
		
		for (int i = 0; i < n ; i++){
			for (int j = 0; j < n; j++) {
				for(int k = 0; k < n ; k++) {
					
					cout << "af::array = Part[" << i*res << ":" <<
					 (i+1)*res-1 << ", "<< j*res << ":" << (j+1)*res-1 <<
					  ", "<< k*res << ":" << (k+1)*res-1 << "]" <<  endl; 
					af::array vol(res, res, res, f32);
					vol =  X(seq(i*res,(i+1)*res-1),seq(j *res,(j+1)*res -1), seq(k*res,(k+1)*res-1));  
					cout << "volume dimensions = " << vol.dims() << endl;
					af::array out;
					if(correlation)
						 out = crossCorrelate(vol, Y);
					else 
						 out = convolve3(vol, Y,AF_CONV_DEFAULT,AF_CONV_AUTO);
					
					//int outdim = out.dims()[0];
					cout << "Max value of chunk convolution = " << max<float>(out) << endl;
					chunk++;
					cout << "done with " << chunk << " chunks" << endl;
					
					// copy out to host
					//float* outHost = out.host<float>(); 
					// switch to the storage device where result is
					//af::setDevice(storageDevice);
					// copy out to the storage device
					//af::array outDevice(outdim, outdim, outdim, f32);
					//outDevice = outHost;
					// add this chunk to the result 
					result(seq(i*res,(i+1)*res-1),seq(j *res,(j+1)*res -1), seq(k*res,(k+1)*res-1)) += out;	
				}
			}
		} 
		return result;
}
*/

