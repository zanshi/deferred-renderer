# Dependencies:
# GLM, GLEW, GLFW

cmake_minimum_required(VERSION 3.4)
set(PROJECT_NAME_STR Deferred)
project(${PROJECT_NAME_STR})


set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/build)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR})
set(PROJECT_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include)
set(PROJECT_CXX_DIR ${PROJECT_SOURCE_DIR}/src)
set(PROJECT_EXT_DIR ${PROJECT_SOURCE_DIR}/external)
set(VENDOR_DIR ${PROJECT_SOURCE_DIR}/vendor)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}-Wall -Wextra -pedantic -std=c++14 ")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_DEBUG} -g")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_RELEASE} -O2")


# Build static libraries
if (NOT APPLE AND NOT UNIX)
    SET(BUILD_SHARED_LIBRARIES OFF)
    SET(CMAKE_EXE_LINKER_FLAGS "-static")
    SET(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
endif(NOT APPLE AND NOT UNIX)

# Add the external module path
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake/modules)

# Find external packages
find_package(PkgConfig REQUIRED)
find_package(OpenGL REQUIRED)
#find_package(GLEW REQUIRED)
#find_package(glfw 3.2 REQUIRED)
#find_package(Assimp REQUIRED)


if (UNIX AND NOT APPLE)
    find_package(X11 REQUIRED)
endif ()

if (APPLE)
    # Find external libraries
    find_library(OPENGL_FRAMEWORK OpenGL)
    find_library(COCOA_FRAMEWORK Cocoa)
    find_library(minizip minizip)
endif (APPLE)

# Add include directories
#include_directories(${OPENGL_INCLUDE_DIR})
#include_directories(${GLEW_INCLUDE_DIRS})
#include_directories(${GLFW3_INCLUDE_PATH})
#include_directories(${ASSIMP_INCLUDE_DIR})


# Our own include directory
set(COMMON_INCLUDES ${PROJECT_SOURCE_DIR}/include)
include_directories(${COMMON_INCLUDES})

# ///////////////////////////////////////////////////
# GLM
include_directories(${VENDOR_DIR}/glm/include)

# ///////////////////////////////////////////////////
# STB
include_directories(${VENDOR_DIR}/stb/include)

# ///////////////////////////////////////////////////
# GLI
include_directories(${VENDOR_DIR}/gli/include)


# ///////////////////////////////////////////////////
# GLEW
add_definitions(-DGLEW_STATIC)

set(GLEW_SOURCE
        ${VENDOR_DIR}/glew/src/glew.c)

set(GLEW_INCLUDE
        ${VENDOR_DIR}/glew/include)

add_library(GLEW STATIC
        ${GLEW_SOURCE}
        ${GLEW_INCLUDE}
        )

target_link_libraries(GLEW
        ${OPENGL_LIBRARY}
        ${EXTRA_LIBS}
        )

include_directories(${GLEW_INCLUDE})

# ///////////////////////////////////////////////////
# GLFW

add_subdirectory(${VENDOR_DIR}/glfw)
set(GLFW_BUILD_EXAMPLES OFF)
set(GLFW_BUILD_TESTS OFF)

set(GLFW_INCLUDE
        ${VENDOR_DIR}/glfw/include)

include_directories(${GLFW_INCLUDE})

# ///////////////////////////////////////////////////
# ZLIB
#add_subdirectory(${VENDOR_DIR}/zlib)
#
#set(ZLIB_INCLUDE
#        ${VENDOR_DIR}/zlib)
#
#include_directories(${ZLIB_INCLUDE})

# ///////////////////////////////////////////////////
# MINIZIP


add_subdirectory(${VENDOR_DIR}/minizip)

#set(MINIZIP_INCLUDE
#        ${VENDOR_DIR}/zlib)

include_directories(${MINIZIP_PUBLIC_HEADERS})

# ///////////////////////////////////////////////////
# ASSIMP
add_subdirectory(${VENDOR_DIR}/assimp)
set(ASSIMP_BUILD_TESTS OFF)
set(ASSIMP_BUILD_SAMPLES OFF)
set(ASSIMP_BUILD_ASSIMP_TOOLS OFF)

set(ASSIMP_INCLUDE
        ${VENDOR_DIR}/assimp/include)

include_directories(${ASSIMP_INCLUDE})

# ///////////////////////////////////////////////////

# Creating our own library
#file(GLOB Application_lib_SOURCES ${PROJECT_SOURCE_DIR}/src/)
#file(GLOB Application_lib_HEADERS ${PROJECT_SOURCE_DIR}/include/*.h)
## We do not want main.cpp in the library
#file(GLOB to_remove ${PROJECT_SOURCE_DIR}/src/main.cpp)
#list(REMOVE_ITEM Application_lib_SOURCES ${to_remove})
#list(REMOVE_ITEM Application_lib_HEADERS ${to_remove})
#add_library(Application_lib ${Application_lib_SOURCES} ${Application_lib_HEADERS} )


# Creating our executable
add_executable(${PROJECT_NAME} src/main.cpp src/Engine.cpp src/DeferredRenderer.cpp src/FBO.cpp
        src/ForwardRenderer.cpp src/GBuffer.cpp src/Mesh.cpp src/Model.cpp src/Quad.cpp src/Shader.cpp )


if (APPLE)
    target_link_libraries(${PROJECT_NAME} ${OPENGL_FRAMEWORK} ${COCOA_FRAMEWORK})
endif (APPLE)

set(OTHER_LIBS ${minizip})

target_link_libraries(${PROJECT_NAME} ${OPENGL_LIBRARIES} ${OTHER_LIBS} GLEW glfw assimp)



