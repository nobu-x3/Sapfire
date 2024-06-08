cmake_minimum_required (VERSION 3.12)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS True)
project(Sandbox VERSION 0.1)

include(engine_translation_units.cmake)

add_subdirectory(engine)
add_subdirectory(sapling)
if(BUILD_TESTS)
add_subdirectory(tests)
endif()

add_executable(Sandbox
        sandbox/src/sandbox_app.cpp
        sandbox/src/sandbox_layer.cpp
        sandbox/src/sandbox_game_context.cpp
        )
target_include_directories(Sandbox PUBLIC engine/include PRIVATE sandbox/include)
target_link_libraries(Sandbox
        Engine
        # spdlog::spdlog
        )
