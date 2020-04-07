# - Example project for accessing SuperNEMO RTD data (Raw Trigger Data)

cmake_minimum_required(VERSION 3.3 FATAL_ERROR)
project(snfee_examples_read_rtd LANGUAGES CXX)

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
  message(STATUS "${PROJECT_NAME} requires an out-of-source build.")
  message(STATUS "Please remove these files from ${CMAKE_BINARY_DIR} first:")
  message(STATUS "  CMakeCache.txt")
  message(STATUS "  CMakeFiles")
  message(STATUS "Once these files are removed, create a separate directory")
  message(STATUS "and run CMake from there, pointing it to:")
  message(STATUS "  ${CMAKE_SOURCE_DIR}")
  message(FATAL_ERROR "in-source build detected")
endif()

# - Load Builtin/Custom Modules
include(GNUInstallDirs)
list(INSERT CMAKE_MODULE_PATH 0 ${PROJECT_SOURCE_DIR}/cmake)
message(STATUS "[info] CMAKE_MODULE_PATH = '${CMAKE_MODULE_PATH}'")

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

target_link_libraries(snfee-rtd-read-calo
  SNFrontEndElectronics::snfee
  )

set_target_properties(snfee-rtd-read-calo
  PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
  ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
  )


# - Executable:
add_executable(snfee-rtd-ana-calo
  rtd_ana_calo.cxx
  calo_histogramming.h
  calo_histogramming.cc
  calo_waveform_fft.h
  calo_waveform_fft.cc
  )

target_link_libraries(snfee-rtd-ana-calo
  SNFrontEndElectronics::snfee
  )

set_target_properties(snfee-rtd-ana-calo
  PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
  ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
  )


install(TARGETS snfee-rtd-read-calo snfee-rtd-ana-calo
  DESTINATION ${CMAKE_INSTALL_BINDIR}
  )
