/*
 * voxelvolume.cpp
 *
 *  Created on: April 13, 2017
 *      Author: nelaturi
 */
 
#include "voxelvolume.hpp"

// vtk stuff for marching cubes and mesh decimation

#include <vtkMarchingCubes.h>
#include <vtkPolyDataConnectivityFilter.h>
#include <vtkPolyDataMapper.h>
#include <vtkActor.h>
#include <vtkProperty.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h> 
#include <vtkDecimatePro.h>

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



vtkSmartPointer<vtkImageData> voxelVolume::create3dVTKImage() {

	///create image data from the host array
	vtkSmartPointer<vtkImageData> imageData = vtkSmartPointer<vtkImageData>::New();
	
	//specify size of image data
	cout << "Image data dimensions = " << dims[0] << ","<< dims[1] << "," << dims[2] << endl;
	
	imageData->SetDimensions(dims[0],dims[1],dims[2]);
#if VTK_MAJOR_VERSION <= 5
	imageData->SetNumberOfScalarComponents(1);
	imageData->SetScalarTypeToDouble();
#else
	imageData->AllocateScalars(VTK_DOUBLE,1);
#endif
	//populate imageData array
	cout << "Copying to imageData and visualizing" << endl;
	for(int k = 0; k < dims[2]; k++){
		for (int j = 0; j < dims[1]; j++){
			for (int i = 0; i < dims[0]; i++){
				double *voxel = static_cast<double*>(imageData->GetScalarPointer(i,j,k));
				voxel[0] =  hostVol[j*dims[0]*dims[1]+i*dims[1]+k] * 255.0;  
			}
		}
	}
	
	return imageData;

}


void voxelVolume::visualizeVolume(float levelSet, float decimation){
	/* Use Marching Cubes and Mesh decimation to visualize the 
	* voxel volume generated on the host. 
	*/
 
	vtkSmartPointer<vtkImageData> imageData = create3dVTKImage();

	// Create a 3D model using marching cubes
	vtkSmartPointer<vtkMarchingCubes> mc =
			vtkSmartPointer<vtkMarchingCubes>::New();
	mc->SetInput(imageData);
	mc->ComputeNormalsOn();
	mc->ComputeGradientsOn();
	mc->SetValue(0, levelSet); 


	vtkSmartPointer<vtkDecimatePro> decimate =
	   vtkSmartPointer<vtkDecimatePro>::New();
	#if VTK_MAJOR_VERSION <= 5
  	decimate->SetInputConnection(mc->GetOutputPort());
	#else
 	 decimate->SetInputData(input);
	#endif
  	//decimate->SetTargetReduction(.99); //99% reduction (if there was 100 triangles, now there will be 1)
  	decimate->SetTargetReduction(decimation); 
  	decimate->Update();

	// To remain largest region
	vtkSmartPointer<vtkPolyDataConnectivityFilter> confilter =
			vtkSmartPointer<vtkPolyDataConnectivityFilter>::New();
	confilter->SetInputConnection(decimate->GetOutputPort());
	confilter->SetExtractionModeToLargestRegion();

	bool extractMaxIsoSurface = false;
	// Create a mapper
	vtkSmartPointer<vtkPolyDataMapper> mapper =
			vtkSmartPointer<vtkPolyDataMapper>::New();
	if (extractMaxIsoSurface)
	{
		mapper->SetInputConnection(confilter->GetOutputPort());
	}
	else
	{
		mapper->SetInputConnection(decimate->GetOutputPort());
	}

	mapper->ScalarVisibilityOff();    // utilize actor's property I set

	// Visualize
	vtkSmartPointer<vtkActor> actor =
			vtkSmartPointer<vtkActor>::New();
	actor->GetProperty()->SetColor(1,1,1);
	actor->SetMapper(mapper);


	vtkSmartPointer<vtkRenderWindow> renWin =
			vtkSmartPointer<vtkRenderWindow>::New();
	vtkSmartPointer<vtkRenderer> ren1 =
			vtkSmartPointer<vtkRenderer>::New();
	ren1->SetBackground(0.1,0.4,0.2);

	ren1->AddViewProp(actor);
	ren1->ResetCamera();
	renWin->AddRenderer(ren1);
	renWin->SetSize(501,500); // intentional odd and NPOT  width/height

	vtkSmartPointer<vtkRenderWindowInteractor> iren =
			vtkSmartPointer<vtkRenderWindowInteractor>::New();
	iren->SetRenderWindow(renWin);

	renWin->Render(); // make sure we have an OpenGL context.
	iren->Start();


}



