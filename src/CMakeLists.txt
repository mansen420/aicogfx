# Collect all C/C++ source files in the engine
file(GLOB_RECURSE ENGINE_SRC CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
)

set(BIN_NAME aicogfx)

# Define the engine library
add_library(${BIN_NAME} STATIC)
target_sources(aicogfx PRIVATE ${ENGINE_SRC})

# Enable warnings
if (MSVC)
    add_compile_options(${BIN_NAME} PRIVATE /W4)
else()
    target_compile_options(${BIN_NAME} PRIVATE -Wall -Wextra -Wpedantic)
endif()

set_target_properties(${BIN_NAME} PROPERTIES EXPORT_COMPILE_COMMANDS ON)

# Set include paths
target_include_directories(${BIN_NAME}
    PUBLIC
        ${CMAKE_SOURCE_DIR}/include
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}  # for internal headers
)

target_compile_definitions(${BIN_NAME} PRIVATE 
    $<$<CONFIG:Debug>:AICO_DEBUG=1>
)
# Link dependencies
find_package(OpenGL REQUIRED)
target_link_libraries(${BIN_NAME} PRIVATE glad glm::glm OpenGL::GL glfw tinyobjloader)
