cmake_minimum_required(VERSION 3.15)

# Set the project name to your project name, my project isn't very descriptive
project(opensmith C CXX)
include(cmake/StandardProjectSettings.cmake)

include_directories(.)
include_directories(include)
include_directories(portaudio/include)

# Link this 'library' to set the c++ standard / compile-time options requested
add_library(project_options INTERFACE)
target_compile_features(project_options INTERFACE cxx_std_17)

# Link this 'library' to use the warnings specified in CompilerWarnings.cmake
add_library(project_warnings INTERFACE)

# standard compiler warnings
include(cmake/CompilerWarnings.cmake)
set_project_warnings(project_warnings)

######################
# conan dependencies #
######################

set(CONAN_EXTRA_REQUIRES "")
set(CONAN_EXTRA_OPTIONS "")

include(cmake/Conan.cmake)
run_conan()

#################
# git clone gli #
#################

include(ExternalProject)

ExternalProject_Add(
    gli
    PREFIX ${PROJECT_SOURCE_DIR}/external/gli
    GIT_REPOSITORY https://github.com/g-truc/gli
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""#${CMAKE_COMMAND} -E copy_directory <SOURCE_DIR>/gli ${PROJECT_SOURCE_DIR}/include/gli
)
ExternalProject_Get_Property(gli source_dir)
set(GliIncludeDir ${source_dir})


message(STATUS "{GliIncludeDir} = ${GliIncludeDir}")

#include_directories(${EXTERNAL_INSTALL_LOCATION}/include)

#############
# portaudio #
#############

#set (PA_USE_ASIO 1)
#set (PA_ENABLE_DEBUG_OUTPUT 0)
add_subdirectory(portaudio EXCLUDE_FROM_ALL)

#########
# build #
#########

add_subdirectory(Rijndael)
add_subdirectory(PsarcReader)
add_subdirectory(Settings)
add_subdirectory(Wem)
add_subdirectory(Audio)
add_subdirectory(Vocals)
add_subdirectory(opensmith)
