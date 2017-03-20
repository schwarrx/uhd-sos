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

#include "helper.h"
// vtk stuff 

using namespace std; 
using namespace af;

af::array read_binvox(string filespec)
{
	// reads a binvox file
	static int version;
	static int depth, height, width;
	static int size;
	static byte *voxels = 0;
	static float tx, ty, tz;
	static float scale;

	ifstream *input = new ifstream(filespec.c_str(), ios::in | ios::binary);

	//
	// read header
	//
	string line;
	*input >> line;  // #binvox
	if (line.compare("#binvox") != 0) {
		cout << "Error: first line reads [" << line << "] instead of [#binvox]" << endl;
		delete input;
		return 0;
	}
	*input >> version;
	//cout << "reading binvox version " << version << endl;

	depth = -1;
	int done = 0;
	while(input->good() && !done) {
		*input >> line;
		if (line.compare("data") == 0) done = 1;
		else if (line.compare("dim") == 0) {
			*input >> depth >> height >> width;
		}
		else if (line.compare("translate") == 0) {
			*input >> tx >> ty >> tz;
		}
		else if (line.compare("scale") == 0) {
			*input >> scale;
		}
		else {
			cout << "  unrecognized keyword [" << line << "], skipping" << endl;
			char c;
			do {  // skip until end of line
				c = input->get();
			} while(input->good() && (c != '\n'));

		}
	}
	if (!done) {
		cout << "  error reading header" << endl;
		return 0;
	}
	if (depth == -1) {
		cout << "  missing dimensions in header" << endl;
		return 0;
	}

	size = width * height * depth;
	voxels = new byte[size];
	if (!voxels) {
		cout << "  error allocating memory" << endl;
		return 0;
	}

	//
	// read voxel data
	//
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
			if (end_index > size) return 0;
			for(int i=index; i < end_index; i++) voxels[i] =value;

			//cout << (float)value << endl;

			if (value) nr_voxels += count;
			index = end_index;
		}  // if file still ok

	}  // while

	input->close();

	af::array A = af::array(width, depth, height, voxels);
	A = A.as(f32);
	//cout << "read " << nr_voxels << " voxels" << endl;
	//cout << "Array dimensions = " << A.dims() << endl;
	//af_print(A);

	return A;

}

