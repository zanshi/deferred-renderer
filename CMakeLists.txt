# Dependencies:
# GLM, GLEW, GLFW

cmake_minimum_required(VERSION 2.8)
set(PROJECT_NAME_STR Tsbk03-project)
project(${PROJECT_NAME_STR})


set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/build)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR})
set(PROJECT_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include)
set(PROJECT_CXX_DIR ${PROJECT_SOURCE_DIR}/src)
set(PROJECT_EXT_DIR ${PROJECT_SOURCE_DIR}/external)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}-Wall -Wextra -pedantic -std=c++11")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_DEBUG} -g")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_RELEASE} -O2")


# Add the external module path
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake/modules)

# Find external packages
find_package(PkgConfig REQUIRED)
find_package(OPENGL REQUIRED)
find_package(GLEW REQUIRED)

pkg_search_module(GLFW REQUIRED glfw3)
pkg_search_module(ASSIMP REQUIRED assimp)

if(UNIX AND NOT APPLE)
  find_package(X11 REQUIRED)
endif()

if(APPLE)
  # Find external libraries
  find_library(OPENGL_FRAMEWORK OpenGL)
  find_library(COCOA_FRAMEWORK Cocoa)
endif(APPLE)

# Add include directories 
include_directories(${GLEW_INCLUDE_DIR})
include_directories(${GLFW_INCLUDE_DIRS})
include_directories(${ASSIMP_INCLUDE_DIRS})


# Our own include directory
set(COMMON_INCLUDES ${PROJECT_SOURCE_DIR}/include)
include_directories(${COMMON_INCLUDES})

#set(GCC_COVERAGE_LINK_FLAGS "-lGLEW -lglfw3 -lGL -lX11 -lXi -lXrandr -lXxf86vm -lXinerama -lXcursor -lrt -lm -pthread")
#SET( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}${GCC_COVERAGE_LINK_FLAGS}" )

# Creating our own library
file(GLOB Application_lib_SOURCES ${PROJECT_SOURCE_DIR}/src/*.cpp)
file(GLOB Application_lib_HEADERS ${PROJECT_SOURCE_DIR}/include/*.h)
# We do not want main.cpp in the library
file(GLOB to_remove ${PROJECT_SOURCE_DIR}/src/main.cpp)
list(REMOVE_ITEM Application_lib_SOURCES ${to_remove})
list(REMOVE_ITEM Application_lib_HEADERS ${to_remove})
add_library(Application_lib ${Application_lib_SOURCES} ${Application_lib_HEADERS})

# Creating our executable
add_executable(${PROJECT_NAME} src/main.cpp)


target_link_libraries(${PROJECT_NAME} Application_lib ${ASSIMP_LIBRARIES} ${OPENGL_LIBRARIES} ${OPENGL_glu_LIBRARY} ${GLEW_LIBRARY} ${GLFW_LIBRARIES} ${CMAKE_CXX_FLAGS})


if(APPLE)
  target_link_libraries(${PROJECT_NAME} ${OPENGL_FRAMEWORK} ${COCOA_FRAMEWORK})
endif(APPLE)