file(GLOB_RECURSE MyCPPs *.cpp)
add_executable(opensmith ${MyCPPs})

add_dependencies(opensmith gli)

target_link_libraries(
  opensmith PRIVATE project_options project_warnings
  CONAN_PKG::glew CONAN_PKG::glfw CONAN_PKG::glm
  Audio Settings Wem PsarcReader Vocals)

if (WIN32)
    target_link_libraries(opensmith PRIVATE CONAN_PKG::dirent)
endif (WIN32)

set_target_properties(opensmith PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin
        DEBUG_POSTFIX "d"
        )
