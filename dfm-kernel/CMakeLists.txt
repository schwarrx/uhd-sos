CMAKE_MINIMUM_REQUIRED(VERSION 3.2)
project(uhdsos_dfm)


message(";; source directory: ${PROJECT_SOURCE_DIR}")
message(";; binary directory: ${PROJECT_BINARY_DIR}")



# Get a list of all of the source files in the directory:
file(GLOB SOURCE
    "*.h"
    "*.cpp"
)

 
ADD_LIBRARY(uhdsos_dfm_lib ${SOURCE})
ADD_EXECUTABLE(uhdsos_dfm main.cpp)


#set (ENV{ArrayFire_DIR}  "/home/nelaturi/arrayfire")
# Find the ArrayFire package.
FIND_PACKAGE(ArrayFire REQUIRED)
# If ArrayFire is found, the following variables will be defined:
#
# ArrayFire_INCLUDE_DIRS    - Location of ArrayFire's include directory.
# ArrayFire_LIBRARIES       - Location of ArrayFire's libraries. This will default
#                             to a GPU backend if one is found.
# ArrayFire_FOUND           - True if ArrayFire has been located
#
# You may provide a hint to where ArrayFire's root directory may be located
# by setting ArrayFire_DIR.
#
# ----------------------------------------------------------------------------
#
# ArrayFire_CPU_FOUND        - True of the ArrayFire CPU library has been found.
# ArrayFire_CPU_LIBRARIES    - Location of ArrayFire's CPU library, if found
# ArrayFire_CUDA_FOUND       - True of the ArrayFire CUDA library has been found.
# ArrayFire_CUDA_LIBRARIES   - Location of ArrayFire's CUDA library, if found
# ArrayFire_OpenCL_FOUND     - True of the ArrayFire OpenCL library has been found.
# ArrayFire_OpenCL_LIBRARIES - Location of ArrayFire's OpenCL library, if found

# Include the ArrayFire hreaders
INCLUDE_DIRECTORIES(${ArrayFire_INCLUDE_DIRS})


	
# Find openCV
#FIND_PACKAGE(OpenCV REQUIRED)
#TARGET_LINK_LIBRARIES( uhdsos_dfm_lib ${OpenCV_LIBS} )

# ArrayFire CUDA backend
FIND_PACKAGE(CUDA)
IF(${ArrayFire_CUDA_FOUND} AND ${CUDA_FOUND})
    # We need to find CUDA and NVVM as transitive linking is disabled on some OSes 
    FIND_LIBRARY( CUDA_NVVM_LIBRARY
      NAMES "nvvm"
      PATH_SUFFIXES "nvvm/lib64" "nvvm/lib"
      PATHS ${CUDA_TOOLKIT_ROOT_DIR}
      DOC "CUDA NVVM Library"
      )
    MESSAGE(STATUS ${CUDA_TOOLKIT_ROOT_DIR})
    MESSAGE(STATUS "ArrayFire CUDA found. Enabling CUDA benchmark")
    
    TARGET_LINK_LIBRARIES(uhdsos_dfm_lib ${ArrayFire_CUDA_LIBRARIES}
        ${CMAKE_THREAD_LIBS_INIT} ${CUDA_LIBRARIES} ${NVVM_LIB} ${CUDA_CUBLAS_LIBRARIES} ${CUDA_cusolver_LIBRARY} ${CUDA_CUFFT_LIBRARIES} )
ENDIF()

find_package(OpenMP REQUIRED)
if(OPENMP_FOUND)
	set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
endif()

# BOOST 
find_package(Boost REQUIRED)
message("Lib[Boost] include directory: ${Boost_INCLUDE_DIR}")
message("Lib[Boost] libraries: ${Boost_LIBRARIES}")
include_directories(${Boost_INCLUDE_DIR}) 

target_link_libraries(uhdsos_dfm uhdsos_dfm_lib)

