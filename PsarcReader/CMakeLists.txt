file(GLOB MyCPPs *.cpp)
add_library(PsarcReader ${MyCPPs})
target_link_libraries(PsarcReader PRIVATE project_options project_warnings CONAN_PKG::zlib Rijndael)
