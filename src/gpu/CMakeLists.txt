file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
)

add_library(aicogpu STATIC)
target_sources(aicogpu PRIVATE ${SRC_FILES})
target_include_directories(aicogpu
    PUBLIC 
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE 
        ${CMAKE_CURRENT_SOURCE_DIR} # internal headers
)
target_link_libraries(aicogpu PRIVATE aicosys aicommon aicore glad glfw)
