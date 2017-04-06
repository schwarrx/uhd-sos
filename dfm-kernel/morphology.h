/*
 * morphology.h
 *
 *  Created on: March 20, 2017
 *      Author: saigopal nelaturi
 */

#ifndef MORPHOLOGY_H_
#define MORPHOLOGY_H_

#include <arrayfire.h>
#include <fftw3.h>

using namespace af;


array indicator(array x);
array sublevel(array x, double measure);
array sublevelComplement(array x, double measure);
array crossCorrelate (array x, array y); // cross-correlation of static x with dynamic y 
array reflect(array x);
array opening(array x, array y); // morphological opening
array crossCorrelateFFTR2C(array x, array y);

#endif /* MORPHOLOGY_H_ */
