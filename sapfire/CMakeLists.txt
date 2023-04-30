cmake_minimum_required(VERSION 3.9)
project(sapfire VERSION 0.0.1 DESCRIPTION "Sapfire Engine")
add_library(sapfire SHARED
src/test.c
src/core/logger.c
src/core/application.c
src/platform/platform_sdl.c
)
find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})
target_link_libraries(sapfire ${SDL2_LIBRARIES})
# set_target_properties(sapfire PROPERTIES VERSION ${PROJECT_VERSION})
# set_target_properties(sapfire PROPERTIES SOVERSION 1)
set_target_properties(sapfire PROPERTIES PUBLIC_HEADER src/sapfire.h)
target_include_directories(sapfire PUBLIC src)
# install(TARGETS sapfire LIBRARY DESTINATION lib
# ARCHIVE DESTINATION lib)