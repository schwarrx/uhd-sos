/*
 * crossCorrelation.cpp
 *
 *  Created on: March 20, 2017
 *      Author: saigopal nelaturi
 */

#include "crossCorrelation.h"
#include "assert.h"



array indicator(array x){
	// returns the support of a scalar field x
	return (x > 0);
}

array sublevel(array x, double measure){
	// returns the sublevel set of a scalar field above a measure
	return (x >= measure-0.0001);
}

double volume(array x){
	// return number of non-zero elements
	array c = count(x);
	//af_print(c);
	return 0;
}

array sublevelComplement(array x, double measure){
	// This the complement of the sub-level sets, useful in spatial planning
	return (x < measure);
}


array reflect(array x){
	// compute the reflection of the shape, use Hermitian symmetry of DFT 
	return real(ifft3(conjg(fft3(x))));
}


array crossCorrelate (array x, array y) { 
	 // Here x is the static function and y is a window that moves over x and measures overlaps 
	return convolve3(x,reflect(y),AF_CONV_EXPAND,AF_CONV_AUTO);
}





