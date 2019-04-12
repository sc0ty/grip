add_library (External fnmatch.c getopt1.c getopt.c)

target_include_directories (External PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
