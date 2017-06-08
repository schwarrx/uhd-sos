/*
 * morphology.h
 *
 *  Created on: March 20, 2017
 *      Author: saigopal nelaturi
 */

#ifndef MORPHOLOGY_HPP_
#define MORPHOLOGY_HPP_

#include <arrayfire.h>
#include <fftw3.h>



af::array indicator(af::array x);
af::array sublevel(af::array x, double measure);
unsigned volume(af::array x);
af::array sublevelComplement(af::array x, double measure);
af::array crossCorrelate(af::array x, af::array y); // cross-correlation of static x with dynamic y
af::array reflect(af::array x);
af::array opening(af::array x, af::array y); // morphological opening
af::array crossCorrelateFFTR2C(af::array x, af::array y);

#endif /* MORPHOLOGY_HPP_ */
