cmake_minimum_required(VERSION 3.10)
project(renderer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

enable_language(OBJCXX)
file(GLOB SRC "src/*.cpp")

add_executable(renderer
    ${SRC}
)


target_sources(renderer PRIVATE
    external/imgui/imgui.cpp
    external/imgui/imgui_draw.cpp
    external/imgui/imgui_tables.cpp
    external/imgui/imgui_widgets.cpp
    external/imgui/backends/imgui_impl_sdl2.cpp
    external/imgui/backends/imgui_impl_opengl3.cpp
)
# ========= Include paths =========
target_include_directories(renderer PRIVATE
    include
    include/rendering
    include/structs
    include/components
    include/components/colliders
    include/components/lights
    include/model_structs
    external/glfw/include
    external/glm/include
    external/assimp/include
    external/stb
    external/sdl2/include
    external/glad/include
    external/imgui
    external/imgui/backends

    include/Boid
)

# ========= Link libraries =========
target_link_libraries(renderer PRIVATE
    ${CMAKE_SOURCE_DIR}/external/glfw/lib/libglfw.3.dylib
    ${CMAKE_SOURCE_DIR}/external/assimp/lib/libassimp.dylib
    ${CMAKE_SOURCE_DIR}/external/sdl2/lib/libSDL2.dylib
)

# ========= macOS frameworks & OpenGL =========
find_package(OpenGL REQUIRED)
target_link_libraries(renderer PRIVATE ${OPENGL_LIBRARIES})

target_link_libraries(renderer PRIVATE
    "-framework Cocoa"
    "-framework IOKit"
    "-framework CoreVideo"
)
# ========= Metal framework =========
find_library(METAL_LIB Metal)
find_library(FOUNDATION_LIB Foundation)
find_library(QUARTZCORE_LIB QuartzCore)

target_link_libraries(renderer PRIVATE
    ${METAL_LIB}
    ${FOUNDATION_LIB}
    ${QUARTZCORE_LIB}
)

# ========= Copy dylibs next to the executable =========
add_custom_command(TARGET renderer POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_SOURCE_DIR}/external/assimp/lib/libassimp.dylib
        ${CMAKE_SOURCE_DIR}/external/glfw/lib/libglfw.3.dylib
        ${CMAKE_SOURCE_DIR}/external/sdl2/lib/libSDL2.dylib
        $<TARGET_FILE_DIR:renderer>
)
