add_library(Rijndael rijndael.c)
target_link_libraries(Rijndael PRIVATE project_options project_warnings)
