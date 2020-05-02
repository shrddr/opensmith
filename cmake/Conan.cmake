macro(run_conan)
# Download automatically, you can also just copy the conan.cmake file
if(NOT EXISTS "${CMAKE_BINARY_DIR}/conan.cmake")
  message(
    STATUS
      "Downloading conan.cmake from https://github.com/conan-io/cmake-conan")
  file(DOWNLOAD "https://github.com/conan-io/cmake-conan/raw/v0.15/conan.cmake"
       "${CMAKE_BINARY_DIR}/conan.cmake")
endif()

include(${CMAKE_BINARY_DIR}/conan.cmake)

conan_add_remote(NAME bincrafters URL
                 https://api.bintray.com/conan/bincrafters/public-conan)

if (WIN32)
  set(CONAN_EXTRA_REQUIRES ${CONAN_EXTRA_REQUIRES} "dirent/1.23.2")
endif (WIN32)

if(UNIX)
  set(CONAN_EXTRA_REQUIRES ${CONAN_EXTRA_REQUIRES} "portaudio/v190600.20161030@bincrafters/stable")
endif(UNIX)

conan_cmake_run(
  REQUIRES
  ${CONAN_EXTRA_REQUIRES}
  catch2/2.11.0
  docopt.cpp/0.6.2
  fmt/6.1.2
  spdlog/1.5.0
  tinyxml2/8.0.0
  ogg/1.3.3@bincrafters/stable
  vorbis/1.3.6
  zlib/1.2.11
  glfw/3.3.2@bincrafters/stable
  glew/2.1.0@bincrafters/stable
  OPTIONS
  ${CONAN_EXTRA_OPTIONS}
  BASIC_SETUP
  NO_OUTPUT_DIRS
  CMAKE_TARGETS # individual targets to link to
  BUILD
  missing)
endmacro()
