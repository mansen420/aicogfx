# Get all test source files
file(GLOB TEST_SRC CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.c"
)
# Loop through all test source files
foreach(TEST_FILE ${TEST_SRC})
    # Extract filename without extension
    get_filename_component(TEST_NAME ${TEST_FILE} NAME_WE)
    set(TEST_NAME sys_${TEST_NAME})

    add_executable(${TEST_NAME} ${TEST_FILE})
    target_link_libraries(${TEST_NAME} PRIVATE aicosys)

    add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
endforeach()
