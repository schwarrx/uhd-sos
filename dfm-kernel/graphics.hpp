/*
 * graphics.hpp
 *
 *  Created on: April 16, 2017
 *      Author: nelaturi
 */

#ifndef GRAPHICS_HPP_
#define GRAPHICS_HPP_

// vtk  
#include <vtkSmartPointer.h>
#include <vtkImageData.h> 
#include <vtkPolyData.h>
#include <iostream>



vtkSmartPointer<vtkImageData> create3dVTKImage(unsigned char* hostVol, int*dims);

void visualizeRenderWindow(vtkSmartPointer<vtkPolyData> polyData);

vtkSmartPointer<vtkPolyData> extractLevelSetAndSimplify(
        vtkSmartPointer<vtkImageData> imageData, float levelSet,
        float decimation);

void writeSTL(vtkSmartPointer<vtkPolyData> polyData, std::string filename);

void visualizeNonManfWithOriginal(vtkSmartPointer<vtkPolyData> nonManf,
        vtkSmartPointer<vtkPolyData> orig);

void visualizeVolumes(vtkSmartPointer<vtkImageData> nonManf,
        vtkSmartPointer<vtkImageData> orig);

void visualizeVolume(vtkSmartPointer<vtkImageData> vol);

#endif // GRAPHICS_HPP_
