add_library(aico INTERFACE)
target_compile_definitions(aico INTERFACE
    $<$<CONFIG:Debug>:AICO_DEBUG=1>
)

# Add subprojects
add_subdirectory(sys)
add_subdirectory(core)
add_subdirectory(gpu)
add_subdirectory(gfx)
target_link_libraries(aico INTERFACE aicommon aicosys aicore aicogpu)
