/*
 * dfm.cpp
 *
 *  Created on: April 14, 2017
 *      Author: nelaturi
 */
 
#include "dfm.hpp" 
#include "morphology.hpp"
#include <cassert>

int getNBlocks(int resultDim, int inputDim) {
	/// return the number of blocks we will compute the convolution for
	int n = inputDim/ resultDim ; // number of blocks per dimension
	int nBlocks = pow(n,3);
	return nBlocks;

}

af::array sliceHostVoxelVolume(int xmin, int xmax, int ymin, int ymax, int zmin, int zmax, voxelVolume* inputVolume) {
	/// return a subset of the host array defined by the indices per dimension.
	/// The subset is returned on the GPU as an arrayfire array.
	
	// we will only operate with voxels of identical dimension
	assert((xmax-xmin) == (ymax-ymin));
	assert((xmax-xmin) == (zmax-zmin));
	
	int dim = (xmax-xmin);
	
	int* dims = inputVolume->getDims();
	
	
	std::vector<byte> hostVolData = inputVolume->getHostVolume();
	
	static int size = pow(dim,3) ;
	std::vector<float> hostSlice = std::vector<float>(size,0);
	int count = 0;
	
	// can we use the fact that hostVolume is a vector and copy by reference?
	cout << "Slicing array " << endl;
	for(int k = zmin; k < zmax; k++){
		for(int j = ymin; j < ymax; j++) {
			for (int i = xmin; i < xmax; i++) {
				hostSlice[count] = (float)(hostVolData[j*dims[0]*dims[1]+i*dims[1]+k]); 
				count++;
			}
		}
	}
	af::array slice = af::array(dim,dim,dim,hostSlice.data());
	cout << "done slicing " << endl;
	
	return slice;
	
}



void processBlocks(int startBlockId, int endBlockId, int inputDim, int selemDim, voxelVolume* inputVolume,af::array selem,  bool correlate) {

		// create a 4d array whose 3d-slices are the blocks to be processed in parallel 
		// on the gpu that is set as the device before calling this function.
		
		int dimRes = inputDim+selemDim-1 ; // the convolution size
		
		int dim = endBlockId-startBlockId;
		
		// before allocating we need to make sure the device has enough memory - TODO
		
		int n = pow(dim,3);
		af::array blocks(dimRes,dimRes,dimRes,n );
		 
		unsigned ij = dim*dim;
		/*
		af::array k(n, s32);
		af::array j(n, s32);
		af::array i(n, s32);
		gfor(seq p, n){
			k(p) = p/ij;
			j(p) = (p %ij)/dim;
			i(p) = (p% ij)%dim;
		}
		cout << "starting parallel gfor " << endl;
		af_print(k);
		af_print(j);
		af_print(i); 
		*/
		
		//gfor(af::seq x, n){
		for (int x = 0; x < n; x++){
			// given a block id, get the block extents as indices to slice the input volume
			int k = x/ij;
			int j = (x%ij)/dim;
			int i = (x%ij)%dim;
			cout << i << "," << j << "," << k << endl;
			std::cout << "af::array = Part[" << i*inputDim<< ":" <<(i+1)*inputDim << "," << j*inputDim << ":" << (j+1)*inputDim <<
					  ", "<< k*inputDim << ":" << (k+1)*inputDim << "]" <<  std::endl;
			af::array block = sliceHostVoxelVolume( i*inputDim, (i+1)*inputDim, j*inputDim, (j+1)*inputDim, k*inputDim, (k+1)*inputDim, inputVolume);
			cout << block.dims() << endl;

			//blocks(af::span, af::span, af::span, x) = crossCorrelate(block, selem);
			crossCorrelate(block,selem);
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
		
		// do this the naive way first
		int chunk = 0;
		 
		af::array result(dim1d, dim1d, dim1d,f32);  
		
		for (int i = 0; i < n ; i++){
			for (int j = 0; j < n; j++) {
				for(int k = 0; k < n ; k++) {
					
					std::cout << "af::array = Part[" << i*res << ":" <<
					 (i+1)*res-1 << ", "<< j*res << ":" << (j+1)*res-1 <<
					  ", "<< k*res << ":" << (k+1)*res-1 << "]" <<  std::endl; 
					af::array vol(res, res, res, f32);
					vol =  X(seq(i*res,(i+1)*res-1),seq(j *res,(j+1)*res -1), seq(k*res,(k+1)*res-1));  
					std::cout << "volume dimensions = " << vol.dims() << std::endl;
					af::array out;
					if(correlation)
						 out = crossCorrelate(vol, Y);
					else 
						 out = convolve3(vol, Y,AF_CONV_DEFAULT,AF_CONV_AUTO);
					
					//int outdim = out.dims()[0];
					std::cout << "Max value of chunk convolution = " << max<float>(out) << std::endl;
					chunk++;
					std::cout << "done with " << chunk << " chunks" << std::endl;
					
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



void dfmAnalysis(std::string binvoxFile, int device)
{
	/** Do a design for manufacturability analysis using morphological 
	* operations implemented by calculating convolutions batched over 
	* multiple GPUs
	*/

	cout << "Setting device .. " ;
	af::setDevice(device);
	cout << "done" <<endl;
	// create the part voxel volume on the host 
	voxelVolume* partHost = new voxelVolume(binvoxFile); 
    int* dims = partHost->getDims();
    cout << "Part dimensions = "<< dims[0] << ", "<< dims[1] << ", " << dims[2] << endl;
    //partHost->visualizeVolume(1,0.1);  // visualize the volume if needed
        
    
    // create a structuring element
    SphereElement<32> sp({15});  
    
    
	//sp.visualizeVolume(1,0.9);
	// create an arrayfire array for the structuring element
	
	int selemDim = 32; // could do this using a getDim but that's not needed
	
	
	cout << "Selem start " << endl;
	
	af::array selem(selemDim, selemDim, selemDim, f32);
	std::vector<byte> selemVolData = sp.getHostVolume();
	cout << "Selem generated" << endl;
	
	
	for(int k = 0; k < selemDim; k++){
		for(int j = 0; j < selemDim; j++) {
			for (int i = 0; i < selemDim; i++) {
				selem(i,j,k) = (float)(selemVolData[j*dims[0]*dims[1]+i*dims[1]+k]); 
			}
		}
	}
	cout << "Created structuring element " << endl;
	
	
	// we want to break up the calculation into blocks of 256^3 
	// and we assume the input is dims[0]^3, i.e. dims[i] = dims[j]
	// i = 0..2, j = 0..2
	int nBlocks = getNBlocks(256,dims[0]) ; 
	std::cout << " We will divide the input volume into " << nBlocks << " blocks" << std::endl;
	
	int nBatches = nBlocks/2;
	for(int i = 0; i < nBatches; i++){
		processBlocks(i,2*(i+1),256,32, partHost, selem,true);
	}
	

}
