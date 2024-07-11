cmake_minimum_required (VERSION 3.12)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS True)
project(Sapling VERSION 0.1)

add_subdirectory(vendor/json)

file(GLOB IMGUI_FILES "vendor/imgui/*.h"
    "vendor/imgui/*.cpp"
    "vendor/imgui/backends/imgui_impl_win32.h"
    "vendor/imgui/backends/imgui_impl_win32.cpp"
    "vendor/imgui/backends/imgui_impl_dx12.h"
    "vendor/imgui/backends/imgui_impl_dx12.cpp"
    "vendor/ImGuiFileDialog/ImGuiFileDialog.cpp"
    "vendor/ImGuiFileDialog/ImGuiFileDialog.h"
    "vendor/ImGuiFileDialog/ImGuiFileDialogConfig.h"
    )

add_executable(Sapling
    src/sapling_app.cpp
    src/sapling_layer.cpp
    src/icons.cpp
    src/rtti_drawer.cpp
    src/subeditors/subeditor.cpp
    src/subeditors/level_editor.cpp
    src/subeditors/material_editor.cpp
    src/widgets/scene_hierarchy.cpp
    src/widgets/entity_inspector.cpp
    src/widgets/scene_view.cpp
    src/widgets/asset_browser.cpp
    src/widgets/material_inspector.cpp
    src/widgets/material_preview_settings.cpp
    src/widgets/tiny_widgets/buttons.cpp
    src/assets/project_reader.cpp
    ${IMGUI_FILES}
    )

target_include_directories(Sapling PUBLIC
    ${ROOT_PATH_ABS}/engine/include
    include
    vendor/imgui
    vendor/ImGuiFileDialog
    vendor/json/include
    )

target_link_libraries(Sapling
    PUBLIC
    Engine
    PRIVATE
    nlohmann_json::nlohmann_json
    )

## Copying dep dlls to sapling
add_custom_command(TARGET Sapling POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:Engine> $<TARGET_FILE_DIR:Sapling>/$<TARGET_FILE_NAME:Engine>
)
add_custom_command(TARGET Sapling POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:spdlog::spdlog> $<TARGET_FILE_DIR:Sapling>/$<TARGET_FILE_NAME:spdlog::spdlog>
)
add_custom_command(TARGET Sapling POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/engine/vendor/dxc/_bin/x64/dxcompiler.dll $<TARGET_FILE_DIR:Sapling>/dxcompiler.dll
)
add_custom_command(TARGET Sapling POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/engine/vendor/dxc/_bin/x64/dxil.dll $<TARGET_FILE_DIR:Sapling>/dxil.dll
)
add_custom_command(TARGET Sapling POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/engine/vendor/DirectXAgilitySDK/_bin/x64/D3D12Core.dll $<TARGET_FILE_DIR:Sapling>/D3D12Core.dll
)
add_custom_command(TARGET Sapling POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/engine/vendor/DirectXAgilitySDK/_bin/x64/d3d12SDKLayers.dll $<TARGET_FILE_DIR:Sapling>/d3d12SDKLayers.dll
)
add_custom_command(TARGET Sapling POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/engine/vendor/DirectXTK12/_Bin/Desktop_2022_Win10/x64/Release/DirectXTK12.lib $<TARGET_FILE_DIR:Sapling>/DirectXTK12.lib
)
add_custom_command(TARGET Sapling POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different ${CMAKE_SOURCE_DIR}/assets $<TARGET_FILE_DIR:Sapling>/assets
)
add_custom_command(TARGET Sapling POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different ${CMAKE_SOURCE_DIR}/editor_assets $<TARGET_FILE_DIR:Sapling>/editor_assets
)
