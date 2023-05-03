cmake_minimum_required(VERSION 3.9)
project(sapfire VERSION 0.0.1 DESCRIPTION "Sapfire Engine")
add_library(sapfire SHARED
src/test.c
src/core/logger.c
src/core/application.c
src/platform/platform_sdl.c
src/core/sfmemory.c
src/containers/vector.c
src/core/event.c
src/core/input.c
src/core/clock.c
src/renderer/renderer.c
src/renderer/renderer_provider.c
src/renderer/vulkan/vulkan_provider.c
src/core/sfstring.c
)
find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})
target_link_libraries(sapfire ${SDL2_LIBRARIES})
find_package(Vulkan REQUIRED)
target_link_libraries(sapfire ${Vulkan_LIBRARIES})
# set_target_properties(sapfire PROPERTIES VERSION ${PROJECT_VERSION})
# set_target_properties(sapfire PROPERTIES SOVERSION 1)
set_target_properties(sapfire PROPERTIES PUBLIC_HEADER src/sapfire.h)
target_include_directories(sapfire PUBLIC src)
# install(TARGETS sapfire LIBRARY DESTINATION lib
# ARCHIVE DESTINATION lib)