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

#include "morphology.h"
#include "helper.h"

using namespace af;
using namespace std;


array getCylinderSelem(double radius, double height, int dim){
	// Return a cylindrical structuring element of radius r and height h
	// in a voxel array with resolution dim per axis
	array selem = array(dim,dim,dim,f32);
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

array getSphereSelem(double radius, int dim){
	// Return a spherical structuring element of radius r in a voxel array 
	// with resolution dim per axis
	array selem = array(dim,dim,dim,f32);
	double center = dim/2; // the arrays are expected to be powers of 2
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

array chunkConvolution(array X, array Y, bool correlation){
	// assuming array x is very large and array y is small 
	// and fits on the gpu, this function breaks up x into 
	// small chunks and computes the convolution 
	// also assume x is a power of 2 three-dimensional array
	
		int res = 256; // we want to break up the input into res^3 chunks
		int dim1d = X.dims()[0];
		int n = dim1d/res;
		int nchunks = pow(n,3);
		cout << " We will divide the input volume into " << nchunks << " chunks" << endl;
		// do this the naive way first
		int chunk = 0;
		array result(dim1d, dim1d, dim1d, f32);
		for (int i = 0; i < n ; i++){
			for (int j = 0; j < n; j++) {
				for(int k = 0; k < n ; k++) {
					
					cout << "Array = Part[" << i*res << ":" <<
					 (i+1)*res-1 << ", "<< j*res << ":" << (j+1)*res-1 <<
					  ", "<< k*res << ":" << (k+1)*res-1 << "]" <<  endl;
					  
					array vol = X(seq(i*res,(i+1)*res-1),seq(j *res,(j+1)*res -1), seq(k*res,(k+1)*res-1));  
					cout << "volume dimensions = " << vol.dims() << endl;
					array out;
					if(correlation)
						 out = crossCorrelate(vol, Y);
					else 
						 out = convolve3(vol, Y,AF_CONV_DEFAULT,AF_CONV_AUTO);
					cout << "Max value of chunk convolution = " << max<float>(out) << endl;
					chunk++;
					cout << "done with " << chunk << " chunks" << endl;
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
		// Select a device and display arrayfire info
		int device = argc > 1 ? atoi(argv[2]) : 0;
		af::setDevice(device);
		af::info();
		af::infoString(true); 
		  
		AF_MEM_INFO("At the beginning=");
  
		// design geometry indicator function 
		array part = read_binvox(argv[1]);  
		cout << "PART DIMENSIONS=" <<  part.dims() << endl;
  	
  		array selem = getSphereSelem(15,32);  // selem or 'structuring element' is the min manf feature.
  		selem = selem.as(f32);
  	
  		///visualize(selem);
  		AF_MEM_INFO("Memory=");
		
		// calculate the overlap measures 
		cout << "starting " << endl; 
		af::timer::start();
		
		// notice that we may have overlaps between the chunks and so we need to handle the boundary 
		// conditions -- limiting the overlap volume to be maximally the measure of the selem does this.
		
		array result = chunkConvolution(part, selem, true); // compute the cross-correlation of part and selem in 'chunks'
		
		
		unsigned mu_selem = volume(selem); // mu is the symbol for measure  
		cout << "Maximum value in correlation = " <<  max<float>(result) << endl;
		volume(sublevel(result, mu_selem));
		array condition = (result > mu_selem).as(f32);
		array correlation  = ((!condition) * result + (condition)*mu_selem);
		cout << "Maximum value in correlation after threshold = " <<  max<float>(result) << endl;
		
		
		// now extract the sublevel set corresponding to full containment
		array feasibleSelemTrans = sublevel(correlation, mu_selem);
		
		// convolve this with the structuring element to get the opening
		result = chunkConvolution(feasibleSelemTrans, selem, false );
		
		 
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
