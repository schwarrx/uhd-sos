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

enum class StructuringElementType: std::int8_t  {Cylinder, Sphere, Field};

template<StructuringElementType EnumVal, int dim>class StructuringElement{
	/** 
	* This class defines a structuring element that is to be
	* used in morphological operations. The structuring element
	* is typically a small 3d array (small relative to the size 
	* of the 3d image being analyzed) and is often a simple shape
	* as decribed in the enum class StructuringElementType above.
	* The Field type is included to represent an arbitrary numeric
	* field as the structuring element, and the Cylinder and Sphere
	* types are indicator functions of a cylinder and sphere.
	*/
	
public:
	StructuringElement(std::initializer_list<double> il) {
		/** For a Cylinder EnumVal the initializer list is going to be 
		* a vector of two doubles {radius, height}. For a Sphere Enumval
		* the initializer list is going to be a vector of one double 
		* radius. Need to figure this out for more complex fields 
		*/ 
		
		//std::cout << dim << std::endl;
		// the EnumVal helps us to do some specialization
		if(EnumVal == StructuringElementType::Cylinder) 
			nslices = il.begin()[1]; // only fill in the array upto the required height
		else
			nslices = dim;
		
		selem = af::array(dim,dim,dim,f32);
		std::cout << selem.dims() << std::endl;
		
		double center = dim/2; // the af::arrays are expected to be powers of 2
		for(int i = 0; i < nslices; i++){ 
			for(int j = 0; j < dim; j++){
				for(int k = 0; k < dim; k++){
					selem(i,j,k) = pow((j-center),2)+ pow((k-center),2); 
					if(EnumVal == StructuringElementType::Sphere)
						selem(i,j,k) += pow((i-center),2);
				}
			}
		}
		
		if(EnumVal == StructuringElementType::Cylinder 
			|| EnumVal == StructuringElementType::Sphere){
			double radius = il.begin()[0];
			selem  = selem <= (pow(radius,2));
		}
		//af_print(selem);
	}
	
	af::array getSelem(){
		return selem;
	}
	
private:
	af::array selem; 
	int nslices;

};
 
#endif /* SELEM_HPP_ */
