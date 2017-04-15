/*
 * selem.hpp
 *
 *  Created on: April 13, 2017
 *      Author: nelaturi
 */
 
#ifndef SELEM_HPP_
#define SELEM_HPP_

#include <cstdint>
#include <initializer_list>
#include <arrayfire.h>
#include "voxelvolume.hpp"

enum class StructuringElementType: std::int8_t  {Cylinder, Sphere, Field};

template<StructuringElementType EnumVal, int dim>class StructuringElement: public voxelVolume {
	/** 
	* This class defines a structuring element that is to be
	* used in morphological operations. The structuring element
	* is typically a small 3d array (small relative to the size 
	* of the 3d image being analyzed) and is often a simple shape
	* as decribed in the enum class StructuringElementType above.
	* The Field type is included to represent an arbitrary numeric
	* field as the structuring element, and the Cylinder and Sphere
	* types are indicator functions of a cylinder and sphere. We also
	* assume the voxel dimensions are dim*dim*dim.
	*/
	
public:
	StructuringElement(std::initializer_list<double> il) : voxelVolume(dim) {
		/** For a Cylinder EnumVal the initializer list is going to be 
		* a vector of two doubles {radius, height}. For a Sphere Enumval
		* the initializer list is going to be a vector of one double 
		* radius. Need to figure this out for more complex fields 
		*/ 
		 
		// the EnumVal helps us to do some specialization
		
		if(EnumVal == StructuringElementType::Field) {
			throw std::exception(); // not supporting this for now.
		}
		
		if(EnumVal == StructuringElementType::Cylinder) 
			nslices = il.begin()[1]; // only fill in the array upto the required height
		else
			nslices = dim;
		
		 
		
		double center = dim/2; // the af::arrays are expected to be powers of 2
		
		for(int i = 0; i < pow(dim,3); i++) {
		
			unsigned int x,y,z;
			index2xyz(i,x,y,z);
			
			float val;
			
			val = pow((y-center),2)+ pow((z-center),2);
			if(EnumVal == StructuringElementType::Sphere)
				val += pow((x-center),2);
			double radius = il.begin()[0];
			hostVol[i] = (byte) (val < radius);
		}
		  
	}
	 
	
private: 
	int nslices;

};

template<int dim>
using CylinderElement = StructuringElement<StructuringElementType::Cylinder, dim>;
template<int dim>
using SphereElement = StructuringElement<StructuringElementType::Sphere, dim>;
 
#endif /* SELEM_HPP_ */
