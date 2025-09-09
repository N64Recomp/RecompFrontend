cmake_minimum_required(VERSION 3.20)

project(recompui)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

file(GLOB API_CXX_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/api/*.cpp)
file(GLOB BASE_CXX_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/base/*.cpp)
file(GLOB COMPOSITES_CXX_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/composites/*.cpp)
file(GLOB CONFIG_CXX_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/config/*.cpp)
file(GLOB CORE_CXX_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/core/*.cpp)
file(GLOB ELEMENTS_CXX_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/elements/*.cpp)
file(GLOB RENDERER_CXX_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/renderer/*.cpp)
file(GLOB RML_ELEMENTS_CXX_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/rml_elements/*.cpp)
file(GLOB RML_HACKS_CXX_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/rml_hacks/*.cpp)
file(GLOB UTIL_CXX_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/util/*.cpp)

if (APPLE)
    set(UTIL_CXX_SOURCES ${UTIL_CXX_SOURCES} ${CMAKE_CURRENT_SOURCE_DIR}/src/util/support_apple.mm)
endif()

# Define the library
add_library(recompui STATIC
    ${API_CXX_SOURCES}
    ${BASE_CXX_SOURCES}
    ${COMPOSITES_CXX_SOURCES}
    ${CONFIG_CXX_SOURCES}
    ${CORE_CXX_SOURCES}
    ${ELEMENTS_CXX_SOURCES}
    ${RENDERER_CXX_SOURCES}
    ${RML_ELEMENTS_CXX_SOURCES}
    ${RML_HACKS_CXX_SOURCES}
    ${UTIL_CXX_SOURCES}
)

# ${PROJECT_SOURCE_DIR}/../lib/RmlUi/Backends/RmlUi_Platform_SDL.cpp

SET(LUNASVG_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
# add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../lib/lunasvg)
SET(RMLUI_SVG_PLUGIN ON CACHE BOOL "" FORCE)
SET(RMLUI_TESTS_ENABLED OFF CACHE BOOL "" FORCE)
SET(LUNASVG_BUILD_STATIC ON CACHE BOOL "" FORCE)
set(RT64_STATIC TRUE)
set(RT64_SDL_WINDOW_VULKAN TRUE)
add_compile_definitions(HLSL_CPU)

# SDL2
    include(FetchContent)

    if (DEFINED ENV{SDL2_VERSION})
        set(SDL2_VERSION $ENV{SDL2_VERSION})
    else()
        set(SDL2_VERSION "2.30.3")
    endif()

    # Fetch SDL2 on windows
    FetchContent_Declare(
        sdl2
        URL https://github.com/libsdl-org/SDL/releases/download/release-${SDL2_VERSION}/SDL2-devel-${SDL2_VERSION}-VC.zip
    )
    FetchContent_MakeAvailable(sdl2)
    target_include_directories(recompui PRIVATE
        ${sdl2_SOURCE_DIR}/include
    )
# END SDL2

# add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../lib/RmlUi)
# target_compile_definitions(rmlui_core PRIVATE LUNASVG_BUILD_STATIC)

target_include_directories(recompui PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/api
    ${CMAKE_CURRENT_SOURCE_DIR}/src/base
    ${CMAKE_CURRENT_SOURCE_DIR}/src/composites
    ${CMAKE_CURRENT_SOURCE_DIR}/src/config
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core
    ${CMAKE_CURRENT_SOURCE_DIR}/src/elements
    ${CMAKE_CURRENT_SOURCE_DIR}/src/renderer
    ${CMAKE_CURRENT_SOURCE_DIR}/src/rml_elements
    ${CMAKE_CURRENT_SOURCE_DIR}/src/rml_hacks
    ${CMAKE_CURRENT_SOURCE_DIR}/src/util
    ${PROJECT_SOURCE_DIR}/../recompinput/include
    ${PROJECT_SOURCE_DIR}/../../N64ModernRuntime/librecomp/include
    ${PROJECT_SOURCE_DIR}/../../N64ModernRuntime/ultramodern/include
    ${PROJECT_SOURCE_DIR}/../../N64ModernRuntime/thirdparty
    ${PROJECT_SOURCE_DIR}/../../N64ModernRuntime/thirdparty/miniz
    ${PROJECT_SOURCE_DIR}/../../N64ModernRuntime/N64Recomp/include
    ${PROJECT_SOURCE_DIR}/../../concurrentqueue
    ${PROJECT_SOURCE_DIR}/../../rt64/include
    ${PROJECT_SOURCE_DIR}/../../rt64/src/contrib
    ${PROJECT_SOURCE_DIR}/../../rt64/src/contrib/hlslpp/include
    ${PROJECT_SOURCE_DIR}/../../rt64/src/contrib/dxc/inc
    ${PROJECT_SOURCE_DIR}/../../rt64/src
    ${PROJECT_SOURCE_DIR}/../../rt64/src/rhi
    ${PROJECT_SOURCE_DIR}/../../rt64/src/render
    ${PROJECT_SOURCE_DIR}/../../rt64/src/contrib/nativefiledialog-extended/src/include
    ${PROJECT_SOURCE_DIR}/../../slot_map/slot_map
    ${PROJECT_SOURCE_DIR}/../../RmlUi/Include
    ${PROJECT_SOURCE_DIR}/../../RmlUi/Backends
    ${PROJECT_SOURCE_DIR}/../../GamepadMotionHelpers
)
target_include_directories(recompui PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/include/recompui"
)


target_compile_options(recompui PRIVATE
#    -Wall
#    -Wextra
    -Wno-unused-parameter
)

if (WIN32)
    add_compile_definitions(NOMINMAX)
endif()

# add_subdirectory(${PROJECT_SOURCE_DIR}/../../N64ModernRuntime/thirdparty/miniz ${CMAKE_CURRENT_BINARY_DIR}/miniz)

target_link_libraries(recompui PUBLIC
    RmlUi::Core
    RmlUi::Debugger
    lunasvg
    miniz
)
