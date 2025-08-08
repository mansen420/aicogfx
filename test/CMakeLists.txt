# Get all test source files
file(GLOB TEST_SRC CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.c"
)
file(GLOB TEST_INCLUDES CONFIGURE_DEPENDS
    "${CMAKE_SOURCE_DIR}/include/*.h"
)
# Loop through all test source files
foreach(TEST_FILE ${TEST_SRC})
    # Extract filename without extension
    get_filename_component(TEST_NAME ${TEST_FILE} NAME_WE)

    # Create a test executable
    add_executable(${TEST_NAME} ${TEST_FILE} ${TEST_INCLUDES})

    # Add compile flags
    if(!MSVC)
        add_compile_options(${TEST_NAME} PRIVATE -Wall -Wextra -Wpedantic)
    endif()

    # Link to your engine library
    target_link_libraries(${TEST_NAME} PRIVATE aicogfx)

   # Set output directory
    set_target_properties(${TEST_NAME} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/tests
    )
endforeach()
