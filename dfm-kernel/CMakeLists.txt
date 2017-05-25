CMAKE_MINIMUM_REQUIRED(VERSION 3.2)
project(uhdsos_dfm)

set(uhdsos_dfm_VERSION_MAJOR 0)
set(uhdsos_dfm_VERSION_MINOR 1)

message(";; source directory: ${PROJECT_SOURCE_DIR}")
message(";; binary directory: ${PROJECT_BINARY_DIR}")



# Get a list of all of the source files in the directory:
file(GLOB SOURCE
    "*.h"
    "*.cpp"
)

 
ADD_LIBRARY(uhdsos_dfm_lib ${SOURCE})
ADD_EXECUTABLE(uhdsos_dfm main.cpp)

 
# Find the ArrayFire package.
FIND_PACKAGE(ArrayFire REQUIRED)  
INCLUDE_DIRECTORIES(${ArrayFire_INCLUDE_DIRS})

 
# ArrayFire CUDA backend
FIND_PACKAGE(CUDA REQUIRED)
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
    
    LIST(APPEND CUDA_NVCC_FLAGS --compiler-options -fno-strict-aliasing -lineinfo -use_fast_math -Xptxas -dlcm=cg)

    TARGET_LINK_LIBRARIES(uhdsos_dfm_lib ${ArrayFire_CUDA_LIBRARIES}
        ${CMAKE_THREAD_LIBS_INIT} ${CUDA_LIBRARIES} ${NVVM_LIB} ${CUDA_CUBLAS_LIBRARIES} ${CUDA_cusolver_LIBRARY} ${CUDA_CUFFT_LIBRARIES} )
ENDIF()

find_package(OpenMP REQUIRED)
if(OPENMP_FOUND)
	set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
endif()

# BOOST 
find_package(Boost REQUIRED )
message("Lib[Boost] include directory: ${Boost_INCLUDE_DIR}")
message("Lib[Boost] libraries: ${Boost_LIBRARIES}")
include_directories(${Boost_INCLUDE_DIR}) 

FIND_PACKAGE(Boost COMPONENTS program_options REQUIRED) 
TARGET_LINK_LIBRARIES(uhdsos_dfm_lib ${Boost_LIBRARIES})

find_package(VTK REQUIRED)
message("Lib[VTK] use file: ${VTK_USE_FILE}")
message("Lib[VTK] include directory: ${VTK_INCLUDE_DIR}")
message("Lib[VTK] libraries: ${VTK_LIBRARIES}")
include(${VTK_USE_FILE})
target_link_libraries(uhdsos_dfm_lib ${VTK_LIBRARIES})


target_link_libraries(uhdsos_dfm uhdsos_dfm_lib)


