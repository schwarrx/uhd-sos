
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

#include "dfm.hpp"
#include "options.hpp"

using namespace std;


int main(int argc, char *argv[])
{
	
	
	try {
	
		// Select a device and display af::arrayfire info 
		
		std::string binvoxFile;
		int device;
		
		bool argsOk = processCommandLine(argc, argv, binvoxFile,device);
		if(!argsOk){
			return 1;
		}
		
		
		dfmAnalysis(binvoxFile, device);
         
       
                           
        /*                   	
		array x1 = x.as(f32) ;
		float *host_x = x1.host<float>(); 
		*/
	
		/*
		af::setDevice(device);
		af::info();
		af::infoString(true); 
		 
		  
		AF_MEM_INFO("At the beginning=");
  
		// design geometry indicator function 
		//byte* voxels = read_binvox(argv[1]);  
		cout << "Copying to device " << device << endl;
		byte* partHost = createHostArray(argv[1]); 
		af::array part = partHost;
		cout << "PART DIMENSIONS=" <<  part.dims() << endl;
		*/
  	
  		//af::setDevice(device);
  		
  		//af::array selem = getSphereSelem(15,32);  // selem or 'structuring element' is the min manf feature.
  
  		
  	
  		//visualize(selem, 1, 0.1);
  		//AF_MEM_INFO("Memory=");
		
		/*
		
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
 		*/

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
