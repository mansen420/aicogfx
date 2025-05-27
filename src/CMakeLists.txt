# Collect all C/C++ source files in the engine
file(GLOB_RECURSE ENGINE_SRC CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
)

set(BIN_NAME aicogfx)

# Define the engine library
add_library(${BIN_NAME} STATIC ${ENGINE_SRC})

# Set include paths
target_include_directories(${BIN_NAME}
    PUBLIC
        ${CMAKE_SOURCE_DIR}/include
    PRIVATE
        ${CMAKE_SOURCE_DIR}/vendor/glm
        ${CMAKE_CURRENT_SOURCE_DIR}  # for internal headers
)

# Link dependencies
find_package(OpenGL REQUIRED)
target_link_libraries(${BIN_NAME} PRIVATE glm::glm)
target_link_libraries(${BIN_NAME} PRIVATE OpenGL::GL)
