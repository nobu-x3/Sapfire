cmake_minimum_required (VERSION 3.12)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS True)
project(Sapling VERSION 0.1)

file(GLOB IMGUI_FILES "vendor/imgui/*.h"
    "vendor/imgui/*.cpp"
    "vendor/imgui/backends/imgui_impl_win32.h"
    "vendor/imgui/backends/imgui_impl_win32.cpp"
    "vendor/imgui/backends/imgui_impl_dx12.h"
    "vendor/imgui/backends/imgui_impl_dx12.cpp"
    )

add_executable(Sapling
    src/sapling_app.cpp
    src/sapling_layer.cpp
    src/icons.cpp
    src/subeditors/subeditor.cpp
    src/subeditors/level_editor.cpp
    src/widgets/scene_hierarchy.cpp
    src/widgets/entity_inspector.cpp
    src/widgets/scene_view.cpp
    src/widgets/asset_browser.cpp
    src/widgets/tiny_widgets/buttons.cpp
    ${IMGUI_FILES}
    )
target_include_directories(Sapling PUBLIC ${ROOT_PATH_ABS}/engine/include include vendor/imgui)
target_link_libraries(Sapling PUBLIC Engine)
