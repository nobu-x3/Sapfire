# Tests
project(Tests VERSION 0.1)
set(TESTS
    memory_tests.cpp
)
include(FetchContent)
FetchContent_Declare(
  googletest
  # Specify the commit you depend on and update it regularly.
  URL https://github.com/google/googletest/archive/5376968f6948923e2411081fd9372e71a59d8e77.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)
enable_testing()
add_executable(Tests
    ${TESTS}
    # ${GUCCIGEDON_TESTS}
)

target_include_directories(Tests PUBLIC engine/include)
# include_directories(${Vulkan_INCLUDE_DIR}
#     ${SDL2_INCLUDE_DIRS}
#     ${GLM_INCLUDE_DIR}
#     ${TINYGLTF_INCLUDE_DIR}
#     ${TINYOBJLOADER_INCLUDE_DIR}
#     ${VULKAN_MEMORY_ALLOCATOR_INCLUDE_DIR}
#     ${VK_BOOTSTRAP_INCLUDE_DIR}
#     ${STB_INCLUDE_DIR}
#     ${SPIRV_REFLECT_INCLUDE_DIR}
#     ${KTX_INCLUDE}
#     )

target_link_libraries(Tests
    gtest_main
    Engine
    )

include(GoogleTest)
gtest_discover_tests(Tests)
