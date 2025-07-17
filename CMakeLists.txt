#----------------------------------------------------------------------------
# Setup the project
cmake_minimum_required(VERSION 3.5 FATAL_ERROR)
project(QmioDetector)

#----------------------------------------------------------------------------
# Find Geant4 package and enable all UI and visualization drivers
find_package(Geant4 REQUIRED ui_all vis_all)

# Check if analysis was found
# if(NOT Geant4_analysis_FOUND)
#     message(FATAL_ERROR "Geant4 analysis component required but not found. Please rebuild Geant4 with -DGEANT4_USE_HDF5=ON")
# endif()

#----------------------------------------------------------------------------
# Include Geant4 environment settings and project headers
include(${Geant4_USE_FILE})
include_directories(${PROJECT_SOURCE_DIR}/include)

#----------------------------------------------------------------------------
# Set cry library path and include directories
set(CRY_PATH "${CMAKE_SOURCE_DIR}/external/cry_v1.7")
include_directories(${CRY_PATH}/src)  
link_directories(${CRY_PATH}/lib)

# Define CRY data directory for the source code
add_definitions(-DCRY_DATA_DIR="${CRY_PATH}/data")

# Verify CRY library exists
if(NOT EXISTS "${CRY_PATH}/lib/libCRY.a" AND NOT EXISTS "${CRY_PATH}/lib/libCRY.so")
    message(FATAL_ERROR "CRY library not found in ${CRY_PATH}/lib")
endif()

if(NOT EXISTS "${CRY_PATH}/src/CRYParticle.h")  
    message(FATAL_ERROR "CRY headers not found in ${CRY_PATH}/src")
endif()

#----------------------------------------------------------------------------
# Locate project source files
file(GLOB sources ${PROJECT_SOURCE_DIR}/src/*.cc)

#----------------------------------------------------------------------------
# Copy macro and CRY setup files to the build directory.
# This allows running the executable directly from the build directory.

set(MACRO_FILES
  run.mac
  vis.mac
  default.cry.setup
)

foreach(file ${MACRO_FILES})
  configure_file(
    ${CMAKE_SOURCE_DIR}/macros/${file}
    ${CMAKE_BINARY_DIR}/${file}
    COPYONLY
  )
endforeach()

#----------------------------------------------------------------------------
# Build the executable and link required libraries
add_executable(sim sim.cc ${sources})
target_link_libraries(sim 
    ${Geant4_LIBRARIES} 
    ${CRY_PATH}/lib/libCRY.a  # Explicit path to the CRY library
)

#----------------------------------------------------------------------------
# Optional target name alias for convenience
add_custom_target(QmioDetector DEPENDS sim)



