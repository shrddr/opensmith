file(GLOB MyCPPs *.cpp)
add_library(Audio ${MyCPPs})
target_link_libraries(Audio PRIVATE project_options project_warnings
    PUBLIC CONAN_PKG::ogg CONAN_PKG::vorbis)

if(WIN32)
    # you can try the conan version but there will be no ASIO support
    #target_link_libraries(Audio PRIVATE CONAN_PKG::portaudio)
    target_link_libraries(Audio PRIVATE portaudio_static)
else()
    target_link_libraries(Audio PRIVATE CONAN_PKG::portaudio)
endif()