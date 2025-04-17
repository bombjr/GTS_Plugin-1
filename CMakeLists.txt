cmake_minimum_required(VERSION 3.21)
message("Using toolchain file ${CMAKE_TOOLCHAIN_FILE}.")

########################################################################################################################
## Define project
########################################################################################################################
project(
	GtsPlugin
	VERSION 3.0.0
	DESCRIPTION "Size Matters Plugin."
	LANGUAGES CXX
)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE INTERNAL "")
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_DEBUG OFF)

add_compile_definitions(_DISABLE_EXTENDED_ALIGNED_STORAGE)
add_compile_definitions(GLM_ENABLE_EXPERIMENTAL)

include(GNUInstallDirs)


# Check if MSVC is the compiler
if(MSVC)
    # Extract the version number of the MSVC compiler
    string(REGEX REPLACE "^([0-9]+)\\.([0-9]+).*" "\\1.\\2" MSVC_VERSION_STRING "${MSVC_VERSION}")
    message(STATUS "Detected MSVC version: ${MSVC_VERSION_STRING}")

    # Check if the MSVC version is at least 14.39
    if(MSVC_VERSION LESS 1939)
        message(FATAL_ERROR "MSVC version 14.39 or higher is required. Detected version: ${MSVC_VERSION_STRING}")
    endif()
else()
    message(FATAL_ERROR "This project requires MSVC")
endif()

configure_file(
	${CMAKE_CURRENT_SOURCE_DIR}/cmake/version.rc.in
	${CMAKE_CURRENT_BINARY_DIR}/version.rc
	@ONLY
)

file(GLOB sources
	src/*.cpp
	src/*/*.cpp
	src/*/*/*.cpp
	src/*/*/*/*.cpp
	${CMAKE_CURRENT_BINARY_DIR}/version.rc
)

source_group(
	TREE ${CMAKE_CURRENT_SOURCE_DIR}
	FILES
	${headers}
	${sources}
)

########################################################################################################################
## Configure target DLL
########################################################################################################################

find_package(Boost REQUIRED)
find_package(CommonLibSSE CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(toml11 CONFIG REQUIRED)
find_package(nlohmann_json 3.2.0 REQUIRED)
find_package(lz4 CONFIG REQUIRED)
find_path(GLUINO_INCLUDE_DIRS "gluino/gluino.h")
find_path(CLIBUTIL_INCLUDE_DIRS "CLibUtil/string.hpp")

include(FetchContent)

FetchContent_Declare(cmake_git_version_tracking                   
  GIT_REPOSITORY https://github.com/andrew-hardin/cmake-git-version-tracking.git
  GIT_TAG 6c0cb87edd029ddfb403a8e24577c144a03605a6
)
FetchContent_MakeAvailable(cmake_git_version_tracking)

FetchContent_Declare(
  boost_ext_reflect
  GIT_REPOSITORY https://github.com/qlibs/reflect
  GIT_SHALLOW ON  # Download the branch without its history
  GIT_TAG        v1.2.5
)
FetchContent_MakeAvailable(boost_ext_reflect)

add_commonlibsse_plugin(${PROJECT_NAME} DECLARATIVE SOURCES ${headers} ${sources})
add_library("${PROJECT_NAME}::${PROJECT_NAME}" ALIAS "${PROJECT_NAME}")

target_include_directories(${PROJECT_NAME}
	PRIVATE
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
	$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/src>
	$<INSTALL_INTERFACE:src>
	${GLM_INCLUDE_DIRS}
	${GLUINO_INCLUDE_DIRS}
	${CLIBUTIL_INCLUDE_DIRS}
	${boost_ext_reflect_SOURCE_DIR}
)

target_link_libraries(${PROJECT_NAME}
	PRIVATE
	Boost::headers
	toml11::toml11
	lz4::lz4
	nlohmann_json::nlohmann_json
	cmake_git_version_tracking
)

target_precompile_headers(${PROJECT_NAME}
	PRIVATE
	src/PCH.hpp
)

# #######################################################################################################################
# # Automatic deployment
# #######################################################################################################################

# Use the new cache variable if it is defined.
if(DEFINED GTS_BUILDPRESET)
  set(BUILD_NAME "${GTS_BUILDPRESET}")
else()
  # Extract the last component of the binary directory (assumed to be like "build-debug", "build-release", etc.)
  get_filename_component(BUILD_FOLDER "${CMAKE_BINARY_DIR}" NAME)

  # Use a regex to extract the suffix after the hyphen.
  # For example, if BUILD_FOLDER is "build-debug", then BUILD_NAME becomes "Debug"
  string(REGEX MATCH "[-]([A-Za-z0-9_]+)$" _match "${BUILD_FOLDER}")
  if(_match)
    # Remove the hyphen to get a clean build name (e.g. "Debug")
    string(REGEX REPLACE "[-]" "" BUILD_NAME "${_match}")
  else()
    # Fallback in case the regex doesn't match.
    set(BUILD_NAME "${CMAKE_BUILD_TYPE}")
  endif()
endif()

message(STATUS "Automatic deployment build name: ${BUILD_NAME}")

# Create the target deployment folder.
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E make_directory
          "${CMAKE_CURRENT_SOURCE_DIR}/contrib/Distribution/Plugin-${BUILD_NAME}/")

# Copy the main DLL.
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy
          $<TARGET_FILE:${PROJECT_NAME}>
          "${CMAKE_CURRENT_SOURCE_DIR}/contrib/Distribution/Plugin-${BUILD_NAME}/")

# Copy the PDB if available for Debug or RelWithDebInfo configurations.
if(CMAKE_BUILD_TYPE MATCHES "Debug|RelWithDebInfo")
  add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
            $<TARGET_PDB_FILE:${PROJECT_NAME}>
            "${CMAKE_CURRENT_SOURCE_DIR}/contrib/Distribution/Plugin-${BUILD_NAME}/")
endif()

# Copy the Fonts folder into a new "GTSPlugin" subfolder in the deployment folder.
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
          "${CMAKE_CURRENT_SOURCE_DIR}/contrib/Distribution/Fonts"
          "${CMAKE_CURRENT_SOURCE_DIR}/contrib/Distribution/Plugin-${BUILD_NAME}/GTSPlugin/Fonts")

# Copy the contents from the config folder.
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
          "${CMAKE_CURRENT_SOURCE_DIR}/contrib/Distribution/Config"
          "${CMAKE_CURRENT_SOURCE_DIR}/contrib/Distribution/Plugin-${BUILD_NAME}/GTSPlugin")

file(GLOB_RECURSE OUTPUT_DLLS "${CMAKE_CURRENT_SOURCE_DIR}/contrib/Distribution/**/*.dll")
file(GLOB_RECURSE OUTPUT_PDBS "${CMAKE_CURRENT_SOURCE_DIR}/contrib/Distribution/**/*.pdb")

set_property(TARGET ${PROJECT_NAME}
  APPEND PROPERTY ADDITIONAL_CLEAN_FILES "${OUTPUT_DLLS}")
set_property(TARGET ${PROJECT_NAME}
  APPEND PROPERTY ADDITIONAL_CLEAN_FILES "${OUTPUT_PDBS}")

# Also copy the PDB and DLL to the directory defined by the GTSPLUGIN_COPY_DIR environment variable,
# if it is defined.
if(DEFINED ENV{GTSPLUGIN_COPY_DIR})
  if(CMAKE_BUILD_TYPE MATCHES "Debug|RelWithDebInfo|Release")
	if(CMAKE_BUILD_TYPE MATCHES "Debug|RelWithDebInfo")
		add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
		  COMMAND ${CMAKE_COMMAND} -E copy
				  $<TARGET_PDB_FILE:${PROJECT_NAME}>
				  "$ENV{GTSPLUGIN_COPY_DIR}/")
	endif()

    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy
              $<TARGET_FILE:${PROJECT_NAME}>
              "$ENV{GTSPLUGIN_COPY_DIR}/")

  endif()
endif()