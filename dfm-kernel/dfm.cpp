/*
 * dfm.cpp
 *
 *  Created on: April 14, 2017
 *      Author: nelaturi
 */
 
#include "dfm.hpp" 
#include "morphology.hpp"
#include <cassert>
#include <thread> 
#include <vector>
#include <cmath>
#include <mutex>

class ThreadPool {
 
public:
 
	template<typename Index, typename Callable>
	static void ParallelFor(Index start, Index end, Callable func) {
		// Estimate number of threads in the pool
		const static unsigned nb_threads_hint = std::thread::hardware_concurrency();
		const static unsigned nb_threads = (nb_threads_hint == 0u ? 8u : nb_threads_hint);
		
		cout << "Number of threads = " << nb_threads << endl;
 
		// Size of a slice for the range functions
		Index n = end - start + 1;
		Index slice = (Index) std::round(n / static_cast<double> (nb_threads));
		slice = std::max(slice, Index(1));
 
		// [Helper] Inner loop
		auto launchRange = [&func] (int k1, int k2) {
			for (Index k = k1; k < k2; k++) {
				func(k);
			}
		};
 
		// Create pool and launch jobs
		std::vector<std::thread> pool;
		pool.reserve(nb_threads);
		Index i1 = start;
		Index i2 = std::min(start + slice, end);
		for (unsigned i = 0; i + 1 < nb_threads && i1 < end; ++i) {
			pool.emplace_back(launchRange, i1, i2);
			i1 = i2;
			i2 = std::min(i2 + slice, end);
		}
		if (i1 < end) {
			pool.emplace_back(launchRange, i1, end);
		}
 
		// Wait for jobs to finish
		for (std::thread &t : pool) {
			if (t.joinable()) {
				t.join();
			}
		}
	}
 
	// Serial version for easy comparison
	template<typename Index, typename Callable>
	static void SequentialFor(Index start, Index end, Callable func) {
		for (Index i = start; i < end; i++) {
			func(i);
		}
	}
 
};

int getNBlocks(int resultDim, int inputDim) {
	/// return the number of blocks we will compute the convolution for
	int n = inputDim/ resultDim ; // number of blocks per dimension
	int nBlocks = pow(n,3);
	return nBlocks;

}

int xyzToIndex (int x, int y, int z, int dim) {
	// return the linear index corresponding to a 3d index
	return (z*dim*dim + y*dim + x) ;
}

int* indexToxyz(int index, int dim){
	/// return the 3d indices corresponding to a 1-d index
	int* indices = new int[3];
	int ij = dim*dim; 
	indices[0] = (index%ij)%dim;
	indices[1] = (index%ij)/dim;
	indices[2] = index/ij;
	return indices;

}

af::array sliceHostVoxelVolume(int xmin, int xmax, int ymin, int ymax, int zmin, int zmax, voxelVolume* inputVolume) {
	/// return a subset of the host array defined by the indices per dimension.
	/// The subset is returned on the GPU as an arrayfire array.
	
	// we will only operate with voxels of identical dimension
	assert((xmax-xmin) == (ymax-ymin));
	assert((xmax-xmin) == (zmax-zmin));
	
	int dim = (xmax-xmin);
	int* dims = inputVolume->getDims();
	//cout << "this block's dimension = " << dim << " within a " << dims[0] << " volume" << endl;
	
	auto hostVolData = inputVolume->getHostVolume();
	
	static int size = pow(dim,3) ;
	std::vector<float> hostSlice = std::vector<float>(size,0);
	int count = 0;
	 
	
	// can we use the fact that hostVolume is a vector and copy by reference?
	//cout << "Slicing array " << endl; 
	// This is VERY slow -- consider boost multidimensional arrrays
	for(int k = zmin; k < zmax; k++){
		for(int j = ymin; j < ymax; j++) {
			for (int i = xmin; i < xmax; i++) {
				hostSlice[count] = (float)(hostVolData[j*dims[0]*dims[1]+i*dims[1]+k]); 
				count++;
			}
		}
	}
	
	af::array slice = af::array(dim,dim,dim,hostSlice.data());
	//cout << "done slicing " << endl;
	
	return slice;
	
}


af::array processBlock(int startBlockId, int nBlocksPerDim, int inputDim, int selemDim, voxelVolume* inputVolume,af::array selem,  bool correlate) {


			int* indices = indexToxyz(startBlockId, nBlocksPerDim);
			int k = indices[2];
			int j = indices[1];
			int i = indices[0];
			cout << i << "," << j << "," << k << endl;
			std::cout << "af::array = Part[" << i*inputDim<< ":" <<(i+1)*inputDim << "," << j*inputDim << ":" << (j+1)*inputDim <<  ", "<< k*inputDim << ":" << (k+1)*inputDim << "]" <<  std::endl;
					  
			af::array block = sliceHostVoxelVolume( i*inputDim, (i+1)*inputDim, j*inputDim, (j+1)*inputDim, k*inputDim, (k+1)*inputDim, inputVolume); 
 
			return crossCorrelate(block,selem); 
		

}

void slicedConvolution( voxelVolume* partHost, af::array selem, int blockSize, bool correlate) {
	/** Compute the convolution/correlation of partHost with selem on a single GPU by breaking 
	* down the partHost into several 3d slices or blocks of size blockDim * blockDim * blockDim.
	* The convolution for each slice is calculated and the output is merged into an ArrayFire array
	* that is stored on the device. There are several optimizations possible for this code to increase speedup
	* TODO - change function signature to take in the device and the StructuringElement generated on the host
	* as inputs so that all GPU related activity takes place within this function.
	*/
	int* dims = partHost->getDims();
    cout << "Part dimensions = "<< dims[0] << ", "<< dims[1] << ", " << dims[2] << endl;
    
	int nBlocks = getNBlocks(blockSize,dims[0]) ; 
	std::cout << " We will divide the input volume into " << nBlocks << " blocks" << std::endl;
	int nBlocksPerDim = dims[0]/blockSize; 
	
	int dimRes = dims[0]+selem.dims()[0]; // dimensions of the result convolution
	cout << "Result dimension = " << dimRes << endl;
	af::array result(dimRes, dimRes, dimRes);
	
	int resBlockSize = blockSize + selem.dims()[0];
	cout <<"Result block size = " << resBlockSize << endl;
	
	cout << "starting " << endl; 
	af::timer::start();
		
//#pragma omp parallel for
	for(int i =0; i < nBlocks; i++){
	//std::mutex critical;
	//ThreadPool::ParallelFor(0, nBlocks, [&] (int i) {
	//	std::lock_guard<std::mutex> lock(critical);
	//gfor(seq i, nBlocks){
		af::array out = processBlock(i,nBlocksPerDim,blockSize,32, partHost, selem,true);
		std::cout << out.dims() << std::endl;
		// now insert this slice into the result
		int* indices = indexToxyz(i, dimRes);
		int z = indices[2];
		int y = indices[1];
		int x = indices[0];
		cout << x*resBlockSize << "," << (x+1)*resBlockSize-1 << endl;
		af::array sl = result(seq(x*resBlockSize,(x+1)*resBlockSize),seq(y *resBlockSize,(y+1)*resBlockSize), seq(z*resBlockSize,(z+1)*resBlockSize)) ;
		cout << sl.dims();
		result(seq(x*resBlockSize,(x+1)*resBlockSize),seq(y *resBlockSize,(y+1)*resBlockSize), seq(z*resBlockSize,(z+1)*resBlockSize)) = out;
		
	};
    cout << "Done computing in  " << af::timer::stop() << " s" <<  endl;

}

af::array morphologicalDFM(voxelVolume* partHost, af::array selem){
	/**  DFM analysis on GPU using pure morphological operations.
	* Note that this approach places a limitation on the size of the
	* structuring element selem (maximum 7x7x7 volume). The approach
	* works for partHosts with 1024^3 resolution. The inputs are 
	* unsigned chars so occupy very little space. 
	*/
	int* dims = partHost->getDims();
	cout << "Part dimensions = "<< dims[0] << ", "<< dims[1] << ", " << dims[2] << endl;
	cout << "Copying volume to GPU " << endl;
	af::array part(dims[0], dims[1], dims[2], (partHost->getHostVolume()).data());
	// calculate the morphological opening
	cout << "starting " << endl; 
	af::timer::start(); 
	af::array open = opening(part, selem);
	cout << "Done computing opening in  " << af::timer::stop() << " s" <<  endl;
	cout << "Computing part - opening " << endl;
	af::timer::start(); 
	af::array out =  part - open;
	cout << "Done computing non manufacturable features in  " << af::timer::stop() << " s" <<  endl;
	cout << "Volume before dfm analysis = " << volume(part)  << endl;
	cout << "Volume after dfm analysis = " << volume(out) << endl;
	cout << "Copying to host and finishing GPU context " << endl;
	
	return out;
	
}


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
    //partHost->visualizeVolume(1,0.1);  // visualize the volume if needed
        
    
    // create a structuring element
    SphereElement<7> sp({4});   
	int selemDim = 7; // could do this using a getDim but that's not needed
	
	
	cout << "Selem start " << endl; 
	auto selemVolData = sp.getHostVolume();
	af::array selem(selemDim, selemDim, selemDim, selemVolData.data());
	selem = selem.as(f32);
	cout << "Selem generated" << endl;
	
	// we want to break up the calculation into blocks 
	//int blockSize = 256;
	
	//slicedConvolution(partHost, selem, blockSize, true);
	morphologicalDFM(partHost, selem);


}





		//af::array blocks(dimRes,dimRes,dimRes,n );
		 
		//unsigned ij = dim*dim;
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
		//for (int x = 0; x < dim; x++){
			// given a block id, get the block extents as indices to slice the input volume
			
			
			
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
