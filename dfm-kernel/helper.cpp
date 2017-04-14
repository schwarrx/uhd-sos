/*
 * helper.cpp
 *
 *  Created on: March 20, 2017
 *      Author: nelaturi
 */



#include <string>
#include <fstream>
#include <iostream>
#include <stdlib.h>

#include <arrayfire.h>

#include "helper.hpp"
// vtk stuff  
#include <vtkSmartPointer.h>
#include <vtkImageData.h>
#include <vtkSmartVolumeMapper.h>
#include <vtkColorTransferFunction.h>
#include <vtkPiecewiseFunction.h>
#include <vtkMarchingCubes.h>
#include <vtkPolyDataConnectivityFilter.h>
#include <vtkPolyDataMapper.h>
#include <vtkActor.h>
#include <vtkProperty.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkVolumeProperty.h>
#include <vtkDecimatePro.h>
using namespace std; 
using namespace af;




void visualize(array x,  float levelSet, float decimation){
	// copy data from device to host and visualize on vtk
	// not sure how to do this visualization directly in AF
	// function also requires the level set and the decimation
	// expressed as a percentage in [0,1]. 
	
	array x1 = x.as(f32) ;
	float *host_x = x1.host<float>(); // copy from device to host - expensive
	dim4 dim = x.dims();
	cout << "done copying from host to device " << endl;
	//create image data
	vtkSmartPointer<vtkImageData> imageData = vtkSmartPointer<vtkImageData>::New();
	//specify size of image data
	imageData->SetDimensions(dim[0],dim[1],dim[2]);
#if VTK_MAJOR_VERSION <= 5
	imageData->SetNumberOfScalarComponents(1);
	imageData->SetScalarTypeToDouble();
#else
	imageData->AllocateScalars(VTK_DOUBLE,1);
#endif
	//populate imageData array
	cout << "Copying to imageData and visualizing" << endl;
	for(int k = 0; k < dim[2]; k++){
		for (int j = 0; j < dim[1]; j++){
			for (int i = 0; i < dim[0]; i++){
				double *voxel = static_cast<double*>(imageData->GetScalarPointer(i,j,k));
				voxel[0] = host_x[j*dim[0]*dim[1]+i*dim[1]+k] * 255.0;
			}
		}
	}

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
