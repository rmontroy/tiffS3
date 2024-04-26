# Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
# SPDX-License-Identifier: Apache-2.0

# Set the minimum required version of CMake for this project.
cmake_minimum_required(VERSION 3.13)

# Set this project's name.
project("tiffS3")

# Set the C++ standard to use to build this target.
set(CMAKE_CXX_STANDARD 11)

# Enable CTest for testing these code examples.
include(CTest)

# All symbols hidden by default.
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN ON)

# Find the AWS SDK for C++ package.
find_package(ZLIB REQUIRED)
find_package(AWSSDK REQUIRED COMPONENTS s3)

file(GLOB HEADERS "*.h")
file(GLOB SOURCES 
  "tiffS3.cpp"
  "context.cpp"
)

# eliminate dead code
add_compile_options($<$<CONFIG:RELEASE>:-ffunction-sections>)
add_compile_options($<$<CONFIG:RELEASE>:-fdata-sections>)
add_link_options($<$<CONFIG:RELEASE>:-Wl,--gc-sections>)

# Build library
add_library(${PROJECT_NAME} ${HEADERS} ${SOURCES} )

target_include_directories(${PROJECT_NAME} PUBLIC
$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include ${AWSSDK_INCLUDE_DIRS}>
$<INSTALL_INTERFACE:include>)
# Export public interface symbols
if(BUILD_SHARED_LIBS)
    target_compile_definitions(${PROJECT_NAME} PRIVATE "TIFFS3_EXPORTS")
endif()
target_link_libraries(${PROJECT_NAME} PRIVATE ${AWSSDK_LINK_LIBRARIES})

if(ENABLE_TESTING)
    # Build executable
    add_executable(run_${PROJECT_NAME} ${PROJECT_NAME}.h run_${PROJECT_NAME}.cpp)

    target_include_directories(run_${PROJECT_NAME} PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include ${AWSSDK_INCLUDE_DIRS}>
        $<INSTALL_INTERFACE:include>)
    target_link_libraries(run_${PROJECT_NAME} PRIVATE ${PROJECT_NAME})
endif()


install(TARGETS ${PROJECT_NAME} EXPORT ${PROJECT_NAME}Targets)
install(
  FILES ${PROJECT_NAME}.h
  DESTINATION include
)

# include(CMakePackageConfigHelpers)
# write_basic_package_version_file(
#   "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}/${PROJECT_NAME}ConfigVersion.cmake"
#   VERSION ${VERSION}
#   COMPATIBILITY AnyNewerVersion
# )

export(EXPORT ${PROJECT_NAME}Targets
  FILE "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}/${PROJECT_NAME}Targets.cmake"
#   NAMESPACE Upstream::
)
configure_file(cmake/${PROJECT_NAME}Config.cmake
  "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}/${PROJECT_NAME}Config.cmake"
  COPYONLY
)

set(ConfigPackageLocation lib/cmake/${PROJECT_NAME})
install(EXPORT ${PROJECT_NAME}Targets
  FILE ${PROJECT_NAME}Targets.cmake
#   NAMESPACE Upstream::
  DESTINATION ${ConfigPackageLocation}
)
install(
  FILES
    cmake/${PROJECT_NAME}Config.cmake
    # "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}/${PROJECT_NAME}ConfigVersion.cmake"
  DESTINATION ${ConfigPackageLocation}
#   COMPONENT Devel
)
