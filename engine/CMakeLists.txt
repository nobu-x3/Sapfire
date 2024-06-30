cmake_minimum_required (VERSION 3.8)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS True)
project(Sapfire VERSION 0.1)
######### SPD LOG ##################
set(no_dev_warnings_backup "$CACHE{CMAKE_SUPPRESS_DEVELOPER_WARNINGS}")
set(CMAKE_SUPPRESS_DEVELOPER_WARNINGS ON CACHE INTERNAL "" FORCE)
set(SPDLOG_BUILD_SHARED ON)
set(SPDLOG_BUILD_EXAMPLE OFF)
set(SPDLOG_BUILD_EXAMPLE_HO OFF)
set(SPDLOG_BUILD_TESTS OFF)
set(SPDLOG_BUILD_TESTS_HO OFF)
set(SPDLOG_BUILD_BENCH OFF)
set(SPDLOG_SANITIZE_ADDRESS ON)
set(SPDLOG_INSTALL OFF)
set(SPDLOG_FMT_EXTERNAL OFF)
set(SPDLOG_FMT_EXTERNAL_HO OFF)
set(SPDLOG_NO_EXCEPTIONS OFF)
set(SPDLOG_BUILD_WARNINGS ON)
set(_SILENCE_ALL_MS_EXT_DEPRECATION_WARNINGS)
add_subdirectory(vendor/spdlog)
target_compile_options(spdlog PRIVATE -w)
##################################
set(CMAKE_SUPPRESS_DEVELOPER_WARNINGS ${no_dev_warnings_backup} CACHE INTERNAL "" FORCE)
####### DirectX-Headers ##########
# add_subdirectory(vendor/DirectX-Headers)
##################################
####### DirectXTK12 ##############
set(DIRECTX_DXC_PATH vendor/dxc/bin/x64/dxc.exe)
add_subdirectory(vendor/DirectXTK12)
##################################
###### D3D12MemoryAllocator ######
add_subdirectory(vendor/D3D12MemoryAllocator)
##################################
###### json ######################
add_subdirectory(vendor/json)
##################################
###### obj ######################
add_subdirectory(vendor/tinyobj)
##################################
add_library(Engine STATIC ${SAPFIRE_TRANSLATION_UNITS})
#target_compile_options(Engine PRIVATE -Wall -Wextra)
target_compile_definitions(Engine PUBLIC SF_BUILD_DLL)
target_include_directories(Engine PUBLIC include
                                        vendor/DirectXAgilitySDK/include
                                        vendor/DirectXAgilitySDK/include/d3dx12
                                        vendor/DirectXTK12/Inc
                                        vendor/D3D12MemoryAllocator/include
                                        vendor/stb
                                        vendor/json/include
                                        vendor/tinyobj
                                        )
target_include_directories(Engine SYSTEM INTERFACE vendor/spdlog/include vendor/dxc/inc)
target_precompile_headers(Engine PRIVATE include/engpch.h)
target_link_libraries(Engine PUBLIC
    spdlog::spdlog
    D3D12.lib
    dxgi.lib
    d3dcompiler.lib
    dxguid.lib
    ${CMAKE_CURRENT_LIST_DIR}/vendor/dxc/lib/x64/dxcompiler.lib
    DirectXTK12
    D3D12MemoryAllocator
    tinyobjloader
    PRIVATE
    nlohmann_json::nlohmann_json
    )
