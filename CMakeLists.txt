# 08.08.2024 12:51:09 Genrated by cbp2cmake V2.5.0 Finale X86 
project(hstex LANGUAGES C)
cmake_minimum_required(VERSION 3.0 FATAL_ERROR)
set (CMAKE_INSTALL_PREFIX /usr )
add_executable(hstex ctools/multiargs.c tools.c hstex.c)
install(TARGETS hstex RUNTIME DESTINATION bin)
