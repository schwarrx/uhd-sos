/*
 * helper.h
 *
 *  Created on: March 20, 2017
 *      Author: nelaturi
 */

#ifndef HELPER_H_
#define HELPER_H_

#include <arrayfire.h>

typedef unsigned char byte;
 
af::array read_binvox(std::string filespec); 
void visualize(af::array x);

#endif /* HELPER_H_ */
