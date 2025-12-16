
add_library(kson_module)

file(GLOB KSON_MODULE_SOURCES "*.cppm")

target_sources(kson_module
    PUBLIC
        FILE_SET CXX_MODULES
        FILES ${KSON_MODULE_SOURCES}
)

if(KSON_WITHOUT_JSON_DEPENDENCY)
    target_compile_definitions(kson_module
        PUBLIC KSON_WITHOUT_JSON_DEPENDENCY
    )
endif()

target_compile_features(kson_module PUBLIC cxx_std_20)

target_include_directories(kson_module PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

add_library(kson::module ALIAS kson_module)

# Installation
install(TARGETS kson_module
    EXPORT ${PROJECT_NAME}Targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    FILE_SET CXX_MODULES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/kson/modules
)