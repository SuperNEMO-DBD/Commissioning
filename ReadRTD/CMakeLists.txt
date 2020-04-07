# Require out-of-source build
if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
  message(STATUS "The project requires an out-of-source build.")
  message(STATUS "Please remove these files from ${CMAKE_BINARY_DIR} first:")
  message(STATUS "  CMakeCache.txt")
  message(STATUS "  CMakeFiles")
  message(STATUS "Once these files are removed, create a separate directory")
  message(STATUS "and run CMake from there, pointing it to:")
  message(STATUS "  ${CMAKE_SOURCE_DIR}")
  message(FATAL_ERROR "in-source build detected")
endif()

# Project setup
cmake_minimum_required(VERSION 3.3 FATAL_ERROR)
project(snfee_examples_read_rtd VERSION 0.1.0)

# - Load Builtin/Custom Modules
include(GNUInstallDirs)
list(INSERT CMAKE_MODULE_PATH 0 ${PROJECT_SOURCE_DIR}/cmake)
message(STATUS "[info] CMAKE_MODULE_PATH = '${CMAKE_MODULE_PATH}'")

# - Build setup
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin")

# - Dependencies
find_package(SNFrontEndElectronics REQUIRED)
message(STATUS  "SNFrontEndElectronics_INCLUDE_DIR=${SNFrontEndElectronics_INCLUDE_DIR}")

# - Executable:
add_executable(snfee-rtd-read-calo
  rtd_read_calo.cxx
  calo_histogramming.h
  calo_histogramming.cc
  calo_waveform_fft.h
  calo_waveform_fft.cc
  )

target_link_libraries(snfee-rtd-read-calo PRIVATE
  SNFrontEndElectronics::snfee
  )

# - Executable:
add_executable(snfee-rtd-ana-calo
  rtd_ana_calo.cxx
  calo_histogramming.h
  calo_histogramming.cc
  calo_waveform_fft.h
  calo_waveform_fft.cc
  )

target_link_libraries(snfee-rtd-ana-calo PRIVATE
  SNFrontEndElectronics::snfee
  )

# - Install if required
install(TARGETS snfee-rtd-read-calo snfee-rtd-ana-calo
  DESTINATION ${CMAKE_INSTALL_BINDIR}
  )
