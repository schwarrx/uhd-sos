/*
 * morphology.cpp
 *
 *  Created on: March 20, 2017
 *      Author: saigopal nelaturi
 */

#include "morphology.hpp"
#include "assert.h"

array indicator(array x) {
    // returns the support of a scalar field x
    return (x > 0);
}

array sublevel(array x, double measure) {
    // returns the sublevel set of a scalar field above a measure
    return (x >= measure - 0.0001);
}

unsigned volume(array x) {
    // return number of non-zero elements
    unsigned nnz = count<unsigned>(x > 0);
    return (nnz);
}

array sublevelComplement(array x, double measure) {
    // This the complement of the sub-level sets, useful in spatial planning
    return (x < measure);
}

array reflect(array x) {
    // compute the reflection of the shape, use Hermitian symmetry of DFT
    return (real(ifft3(conjg(fft3(x)))));
}

array crossCorrelate(array x, array y) {
    // Here x is the static function and y is a window that moves over x and measures overlaps
    return (convolve3(x, reflect(y), AF_CONV_EXPAND, AF_CONV_AUTO));
}

array opening(array x, array y) {
    return (dilate3(erode3(x, y), y));
}

array crossCorrelateFFTR2C(array x, array y) {
    // Use explicit invocation of R2C fft
    //dim4 xdims = x.dims();
    //dim4 ydims = y.dims();
    //const dim_t pad0  = xdims[0] + ydims[0];
    //const dim_t pad1  = xdims[1] + ydims[1];
    //const dim_t pad2  = xdims[2] + ydims[2];
    fft3InPlace(x);
    return (x);

}
