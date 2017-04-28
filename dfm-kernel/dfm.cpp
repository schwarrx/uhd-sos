/*
 * dfm.cpp
 *
 *  Created on: April 14, 2017
 *      Author: nelaturi
 */

#include "dfm.hpp" 
#include "morphology.hpp"
#include <cassert>
#include <thread> 
#include <vector>
#include <cmath>
#include <mutex>
#include <af/macros.h>

af::array morphologicalDFM(voxelVolume* partHost, af::array selem) {
    /**  DFM analysis on GPU using pure morphological operations.
     * Note that this approach places a limitation on the size of the
     * structuring element selem (maximum 7x7x7 volume). The approach
     * works for partHosts with 1024^3 resolution. The inputs are
     * unsigned chars so occupy very little space.
     */
    int* dims = partHost->getDims();
    cout << "Part dimensions = " << dims[0] << ", " << dims[1] << ", "
            << dims[2] << endl;
    cout << "Copying volume to GPU " << endl;
    af::array part(dims[0], dims[1], dims[2],
            (partHost->getHostVolume()).data());
    // calculate the morphological opening
    cout << "starting " << endl;
    af::timer::start();
    af::array open = opening(part, selem);
    cout << "Done computing opening in  " << af::timer::stop() << " s" << endl;
    cout << "Computing part - opening " << endl;
    af::timer::start();
    af::array out = part - open;
    cout << "Done computing non manufacturable features in  "
            << af::timer::stop() << " s" << endl;
    cout << "Volume before dfm analysis = " << volume(part) << endl;
    cout << "Volume after dfm analysis = " << volume(out) << endl;

    AF_MEM_INFO("Memory when all arrays are on device=");

    return (out);

}

void dfmAnalysis(std::string binvoxFile, int device) {
    /** Do a design for manufacturability analysis using morphological
     * operations implemented by calculating convolutions batched over
     * multiple GPUs
     */

    cout << "Setting device .. ";
    af::setDevice(device);
    af::info();
    cout << "done" << endl;

    // create a structuring element
    SphereElement<7> sp( { 7 });
    //CylinderElement<7> sp({2,2});
    int selemDim = 7; // could do this using a getDim but that's not needed

    cout << "Selem start " << endl;
    auto selemVolData = sp.getHostVolume();
    //sp.visualizeVolume(3, 0);
    af::array selem(selemDim, selemDim, selemDim, selemVolData.data());
    selem = selem.as(f32);

    // create the part voxel volume on the host
    voxelVolume* partHost = new voxelVolume(binvoxFile);
    //partHost->visualizeVolume(1, 0.1);  // visualize the volume if needed

    cout << "Selem generated" << endl;

    af::array nonManufacturable = morphologicalDFM(partHost, selem);
    //cout << "Writing STL file output " << endl;

    visualizeVolumes(
            create3dVTKImage((nonManufacturable.host<unsigned char>()),
                    partHost->getDims()),
            create3dVTKImage(((partHost->getHostVolume())).data(),
                    partHost->getDims()));

    /* vtkSmartPointer<vtkPolyData> orig = extractLevelSetAndSimplify(
     create3dVTKImage(((partHost->getHostVolume())).data(),
     partHost->getDims()), 1, 0);


     vtkSmartPointer<vtkPolyData> nonManf = extractLevelSetAndSimplify(
     create3dVTKImage((nonManufacturable.host<unsigned char>()),
     partHost->getDims()), 1, 0);

     visualizeNonManfWithOriginal(nonManf, orig);*/
    cout << "Cleaning up " << endl;

}
