file(GLOB MyCPPs *.cpp)
add_library(Wem ${MyCPPs})
target_link_libraries(Wem PRIVATE project_options project_warnings Audio)
