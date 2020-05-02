file(GLOB MyCPPs *.cpp)
add_library(Vocals ${MyCPPs})
target_link_libraries(Vocals PRIVATE project_options project_warnings CONAN_PKG::tinyxml2)
