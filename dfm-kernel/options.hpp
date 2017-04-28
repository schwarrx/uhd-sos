/*
 * options.hpp
 *
 *  Created on: April 13, 2017
 *      Author: nelaturi
 */

#ifndef OPTIONS_HPP_
#define OPTIONS_HPP_

#include <boost/program_options.hpp>
#include <iostream>
#include <sstream>
namespace po = boost::program_options;

bool processCommandLine(int argc, char** argv, std::string& voxels,
        int& device) {

    try {
        po::options_description desc("Program Usage", 1024, 512);
        desc.add_options()("help,h", "produce help message")("device,d",
                po::value<int>(&device)->required(),
                "set the main GPU device - typically the one with max RAM")(
                "voxels,v", po::value<std::string>(&voxels)->required(),
                "set the design voxel model");

        po::variables_map vm;
        po::store(po::parse_command_line(argc, argv, desc), vm);

        if (vm.count("help")) {
            std::cout << desc << "\n";
            return (false);
        }

        // There must be an easy way to handle the relationship between the
        // option "help" and "host"-"port"-"config"
        // Yes, the magic is putting the po::notify after "help" option check
        po::notify(vm);
    } catch (std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return (false);
    } catch (...) {
        std::cerr << "Unknown error!" << "\n";
        return (false);
    }

    return (true);
}

#endif /* OPTIONS_HPP_ */
