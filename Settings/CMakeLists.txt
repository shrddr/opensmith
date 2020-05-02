file(GLOB MyCPPs *.cpp)
add_library(Settings ${MyCPPs})
target_link_libraries(Settings PRIVATE project_options project_warnings)
