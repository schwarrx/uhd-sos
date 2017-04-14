/*******************************************************
 * Author - S Nelaturi
 * Testing arrayfire based cross correlation
 ********************************************************/
 

#include <stdio.h>
#include <arrayfire.h>
#include <cstdio>
#include <cstdlib>
#include <assert.h> 
#include <af/macros.h>

#include "morphology.hpp"
#include "helper.hpp"
#include "selem.hpp"

#include "boost/multi_array.hpp" 

using namespace std;

typedef boost::multi_array<double, 3> hostArray;
typedef hostArray::index index;

namespace 
{ 
  const size_t ERROR_IN_COMMAND_LINE = 1; 
  const size_t SUCCESS = 0; 
  const size_t ERROR_UNHANDLED_EXCEPTION = 2; 
 
} // namespace 

af::array getCylinderSelem(double radius, double height, int dim){
	// Return a cylindrical structuring element of radius r and height h
	// in a voxel af::array with resolution dim per axis
	af::array selem = af::array(dim,dim,dim,f32);
	double center = dim/2; 
	gfor(seq i, dim){
		for(int j = 0; j < dim; j++){
			for(int k = 0; k < dim; k++){
				selem(i,j,k) = pow((j-center),2)+ pow((k-center),2);  
			}
		}
	}
	selem  = selem <= (pow(radius,2));
	return selem;

}

af::array getSphereSelem(double radius, int dim){
	// Return a spherical structuring element of radius r in a voxel af::array 
	// with resolution dim per axis
	af::array selem = af::array(dim,dim,dim,f32);
	double center = dim/2; // the af::arrays are expected to be powers of 2
	gfor(seq i, dim){ 
		for(int j = 0; j < dim; j++){
			for(int k = 0; k < dim; k++){
				selem(i,j,k) = pow((i-center),2)+pow((j-center),2)+ pow((k-center),2); 
			}
		}
	}
	selem  = selem <= (pow(radius,2));
	//print("structuring element =",selem);
	return selem;
}

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




int main(int argc, char *argv[])
{
	try {
	
		// handle input error
		
		
		// Select a device and display af::arrayfire info
		int device = argc > 1 ? atoi(argv[2]) : 0;
		af::setDevice(device);
		af::info();
		af::infoString(true); 
		
		int storageDevice = 7;
		  
		AF_MEM_INFO("At the beginning=");
  
		// design geometry indicator function 
		//byte* voxels = read_binvox(argv[1]);  
		cout << "Copying to device " << device << endl;
		af::array part = read_binvox(argv[1]); 
		part = part.as(f32);
		cout << "PART DIMENSIONS=" <<  part.dims() << endl;
  	
  		StructuringElement<StructuringElementType::Sphere,32> sp = {15};
  		//af::array selem = getSphereSelem(15,32);  // selem or 'structuring element' is the min manf feature.
  		af::array selem = sp.getSelem();
  	
  		visualize(selem, 0,1);
  		AF_MEM_INFO("Memory=");
		
		// calculate the overlap measures 
		cout << "starting " << endl; 
		af::timer::start();
		
		// notice that we may have overlaps between the chunks and so we need to handle the boundary 
		// conditions -- limiting the overlap volume to be maximally the measure of the selem does this.
		
		af::array result = chunkConvolution(part, selem, true, device, storageDevice); // compute the cross-correlation of part and selem in 'chunks'
		
		
		unsigned mu_selem = volume(selem); // mu is the symbol for measure  
		cout << "Maximum value in correlation = " <<  max<float>(result) << endl;
		volume(sublevel(result, mu_selem));
		af::array condition = (result > mu_selem).as(f32);
		af::array correlation  = ((!condition) * result + (condition)*mu_selem);
		cout << "Maximum value in correlation after threshold = " <<  max<float>(result) << endl;
		
		
		// now extract the sublevel set corresponding to full containment
		af::array feasibleSelemTrans = sublevel(correlation, mu_selem);
		
		// convolve this with the structuring element to get the opening
		result = chunkConvolution(feasibleSelemTrans, selem, false, device, storageDevice );
		
		 
        cout << "Done computing in  " << af::timer::stop() << " s" <<  endl;
 	 
 	 	
 	 	AF_MEM_INFO("Memory=");
 		// visualize the result
 		visualize(result, 1, 0.1);

	} catch (af::exception& e) {
		fprintf(stderr, "%s\n", e.what());
		throw;
	}

#ifdef WIN32 // pause in Windows
	if (!(argc == 2 && argv[1][0] == '-')) {
		printf("hit [enter]...");
		fflush(stdout);
		getchar();
	}
#endif
	return 0;
}
