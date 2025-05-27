# Get all test source files
file(GLOB TEST_SRC CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.c"
)

# Loop through all test source files
foreach(TEST_FILE ${TEST_SRC})
    # Extract filename without extension
    get_filename_component(TEST_NAME ${TEST_FILE} NAME_WE)

    # Create a test executable
    add_executable(${TEST_NAME} ${TEST_FILE})

    # Link to your engine library
    target_link_libraries(${TEST_NAME}
        PRIVATE
            ${BIN_NAME}
    )

    # Include engine headers
    target_include_directories(${TEST_NAME}
        PRIVATE
            ${CMAKE_SOURCE_DIR}/include
    )

    # Set output directory
    set_target_properties(${TEST_NAME} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/tests
    )
endforeach()
