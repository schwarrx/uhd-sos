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



int main(int argc, char *argv[])
{
	try {
		// Select a device and display arrayfire info
		int device = argc > 1 ? atoi(argv[2]) : 0;
		af::setDevice(device);
		af::info();
		af::infoString(true); 
		
		AF_MEM_INFO("At the beginning=");
  
		// design geometry indicator function 
		array part = read_binvox(argv[1]); 
		cout << "PART DIMENSIONS=" <<  part.dims() << endl;
  	
  		AF_MEM_INFO("Memory=");
		
		// calculate the overlap measures 
		cout << "starting " << endl; 
		array minManufacturableFeature = array(7,7,7, f32); 
		af::timer::start();  
		array erosion = opening(part, minManufacturableFeature); 
        cout << "Done computing in  " << af::timer::stop() << " s" <<  endl;
 

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
