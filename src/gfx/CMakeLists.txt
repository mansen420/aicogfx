file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
)

add_library(aicogfx STATIC)
target_sources(aicogfx PRIVATE ${SRC_FILES})
target_include_directories(aicogfx
    PUBLIC 
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE 
        ${CMAKE_CURRENT_SOURCE_DIR} # internal headers
)
target_link_libraries(aicogfx PRIVATE aicosys aicommon aicore aicogpu glad glfw tinyobjloader)
