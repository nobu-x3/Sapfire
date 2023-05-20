cmake_minimum_required(VERSION 3.9)
project(sapfire VERSION 0.0.1 DESCRIPTION "Sapfire Engine")
if(MSCV)
# add msvc VAL errors
else()
  add_compile_options(-Werror=vla)
endif()
add_library(sapfire SHARED
src/core/logger.c
src/core/application.c
src/platform/platform_sdl.c
src/platform/filesystem.c
src/core/sfmemory.c
src/containers/vector.c
src/core/event.c
src/core/input.c
src/core/clock.c
src/renderer/renderer.c
src/renderer/renderer_provider.c
src/renderer/vulkan/vulkan_provider.c
src/core/sfstring.c
src/renderer/vulkan/vulkan_device.c
src/renderer/vulkan/vulkan_image.c
src/renderer/vulkan/vulkan_swapchain.c
src/renderer/vulkan/vulkan_render_pass.c
src/renderer/vulkan/vulkan_command_buffer.c
src/renderer/vulkan/vulkan_framebuffer.c
src/renderer/vulkan/vulkan_fence.c
src/renderer/vulkan/vulkan_shader_module.c
src/renderer/vulkan/vulkan_shader.c
src/renderer/vulkan/vulkan_pipeline.c
src/renderer/vulkan/vulkan_buffer.c
src/math/sfmath.c
src/memory/lin_alloc.c
)
find_package(SDL2 REQUIRED)
find_package(Vulkan REQUIRED)
target_link_libraries(sapfire ${SDL2_LIBRARIES} ${Vulkan_LIBRARY})
include_directories(${Vulkan_INCLUDE_DIR} ${SDL2_INCLUDE_DIRS})
# set_target_properties(sapfire PROPERTIES VERSION ${PROJECT_VERSION})
# set_target_properties(sapfire PROPERTIES SOVERSION 1)
set_target_properties(sapfire PROPERTIES PUBLIC_HEADER src/sapfire.h)
target_include_directories(sapfire PUBLIC src)
# install(TARGETS sapfire LIBRARY DESTINATION lib
# ARCHIVE DESTINATION lib)
