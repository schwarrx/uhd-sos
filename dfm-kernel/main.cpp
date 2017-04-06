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

array getSphereSelem(double radius, int dim){
	// Return a spherical structuring element of radius r
	array selem = array(dim,dim,dim,f32);
	double center = dim/2 +0.5; // the arrays are expected to be powers of 2
	gfor(seq i, dim){ 
		for(int j = 0; j < dim; j++){
			for(int k = 0; k < dim; k++){
				selem(i,j,k) = (i-center)*(i-center)+(j-center)* (j-center) + (k-center) * (k-center); 
			}
		}
	}
	selem  = selem > (radius * radius);
	//print("structuring element =",selem);
	return selem;
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
  	
  		array minManufacturableFeature = getSphereSelem(15,32);  
  		minManufacturableFeature = minManufacturableFeature.as(f32);
  	
  		AF_MEM_INFO("Memory=");
		
		// calculate the overlap measures 
		cout << "starting " << endl; 
		af::timer::start();
		int res = 256; // we want to break up the input into res^3 chunks
		int dim1d = part.dims()[0];
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
					  
					array vol = part(seq(i*res,(i+1)*res-1),seq(j *res,(j+1)*res -1), seq(k*res,(k+1)*res-1)); 
					cout << "volume dimensions = " << vol.dims() << endl;
					array out = crossCorrelate(vol, minManufacturableFeature);
					chunk++;
					cout << "done with " << chunk << " chunks" << endl;
					result(seq(i*res,(i+1)*res-1),seq(j *res,(j+1)*res -1), seq(k*res,(k+1)*res-1)) = vol;
					
				}
			}
		} 
		// now handle boundaries between chunks 
		
        cout << "Done computing in  " << af::timer::stop() << " s" <<  endl;
 	 
 	 	
 	 	AF_MEM_INFO("Memory=");
 		// visualize the result
 		visualize(result);

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
