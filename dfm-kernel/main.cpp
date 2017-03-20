/*******************************************************
 * Author - S Nelaturi
 * Testing arrayfire based cross correlation
 ********************************************************/
 

#include <stdio.h>
#include <arrayfire.h>
#include <cstdio>
#include <cstdlib>
#include <assert.h> 

#include "crossCorrelation.h"
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
  
		// design geometry indicator function 
		array part = read_binvox(argv[1]);
		///////int partDim = part.dims()[0];
  
		
		// calculate the overlap measures
		////////int rvDim = partDim + tDim -1;
		cout << "starting " << endl;
		
		af::timer::start(); 


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
