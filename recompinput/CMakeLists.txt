cmake_minimum_required(VERSION 3.20)

project(recompinput)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

file(GLOB CXX_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

# Define the library
add_library(recompinput STATIC
    ${CXX_SOURCES}
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
    target_include_directories(recompinput PRIVATE
        ${sdl2_SOURCE_DIR}/include
    )
# END SDL2

# add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../lib/RmlUi)
# target_compile_definitions(rmlui_core PRIVATE LUNASVG_BUILD_STATIC)

target_include_directories(recompinput PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${PROJECT_SOURCE_DIR}/../recompui/include
    ${PROJECT_SOURCE_DIR}/../recompui/src
    ${PROJECT_SOURCE_DIR}/../../N64ModernRuntime/librecomp/include
    ${PROJECT_SOURCE_DIR}/../../N64ModernRuntime/ultramodern/include
    ${PROJECT_SOURCE_DIR}/../../N64ModernRuntime/thirdparty
    ${PROJECT_SOURCE_DIR}/../../N64ModernRuntime/thirdparty/miniz
    ${PROJECT_SOURCE_DIR}/../../N64ModernRuntime/N64Recomp/include
    ${PROJECT_SOURCE_DIR}/../../GamepadMotionHelpers
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
)
target_include_directories(recompinput PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/include/recompinput"
)


target_compile_options(recompinput PRIVATE
#    -Wall
#    -Wextra
    -Wno-unused-parameter
)

if (WIN32)
    add_compile_definitions(NOMINMAX)
endif()

target_link_libraries(recompinput PUBLIC)
# /c/Program\ Files/Microsoft\ Visual\ Studio/2022/Community/Common7/IDE/CommonExtensions/Microsoft/CMake/CMake/bin/cmake.exe
# /c/Program\ Files/Microsoft\ Visual\ Studio/2022/Community/Common7/IDE/CommonExtensions/Microsoft/CMake/CMake/bin/cmake.exe -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_COMPILER_LAUNCHER=ccache -DCMAKE_C_COMPILER_LAUNCHER=ccache -DCMAKE_CXX_COMPILER=clang-cl -DCMAKE_C_COMPILER=clang-cl -DCMAKE_MAKE_PROGRAM=ninja -G Ninja -S . -B build -DCMAKE_CXX_FLAGS="-Xclang -fexceptions -Xclang -fcxx-exceptions"
