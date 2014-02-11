cmake_minimum_required(VERSION 2.8)
 
if(POLICY CMP0020)
  cmake_policy(SET CMP0020 NEW)
endif()
 
set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} "C:/Program Files\ \(x86\)/Windows\ Kits/8.1/Lib/winv6.3/um/x64")

PROJECT(MatlabQtVTK_GUI)

set(VTK_DIR C:/Programs/VTK/VTK_6.1_VC2013_Build)
find_package(VTK REQUIRED)
include(${VTK_USE_FILE})
 
# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)
find_package(Qt5Widgets REQUIRED QUIET)
 
include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})
 
file(GLOB UI_FILES *.ui)
file(GLOB QT_WRAP *.h)
file(GLOB CPP_FILES *.cpp)
 
qt5_wrap_ui(UISrcs ${UI_FILES})

# CMAKE_AUTOMOC in ON so the MocHdrs will be automatically wrapped.

add_executable(MatlabQtVTK_GUI ${CPP_FILES} ${UISrcs} ${QT_WRAP})

qt5_use_modules(MatlabQtVTK_GUI Core Gui Network)

#set(VTKLIB_DIR "C:/Programs/VTK/VTK_6.1_VC_Build/lib")

#message(STATUS ${VTKLIB_DIR}/${VTK_LIBRARIES})

#target_link_libraries(MatlabQtVTK_GUI ${VTKLIB_DIR}/${VTK_LIBRARIES}) 

target_link_libraries(MatlabQtVTK_GUI ${VTK_LIBRARIES}) 