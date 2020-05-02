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

# Set up some extra Conan dependencies based on our needs
# before loading Conan
set(CONAN_EXTRA_REQUIRES "")
set(CONAN_EXTRA_OPTIONS "")

include(cmake/Conan.cmake)
run_conan()

#set (PA_USE_ASIO 1)
#set (PA_ENABLE_DEBUG_OUTPUT 0)
add_subdirectory(portaudio EXCLUDE_FROM_ALL)

add_subdirectory(Rijndael)
add_subdirectory(PsarcReader)
add_subdirectory(Settings)
add_subdirectory(Wem)
add_subdirectory(Audio)
add_subdirectory(Vocals)
add_subdirectory(opensmith)
