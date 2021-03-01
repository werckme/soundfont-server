set (SOURCES 
    sf3/myfile.cpp
    sf3/mysysinfo.cpp
    sf3/mystring.cpp
    sf3/sfont.cpp
)

if(${USE_EMSCRIPTEN})
    add_executable(sfcompose sfcompose.cpp  ${SOURCES})
    set_target_properties(sfcompose PROPERTIES LINK_FLAGS "-Oz -s MODULARIZE=1 -s EXPORT_NAME=\"startSfCompose\" -s DISABLE_EXCEPTION_CATCHING=0 -s EXTRA_EXPORTED_RUNTIME_METHODS=\"['cwrap', 'UTF8ToString', 'FS']\" -s EXPORTED_FUNCTIONS=\"['_main', '_composejs']\"")
    install(FILES ${CMAKE_BINARY_DIR}/src/sfcompose.wasm DESTINATION .)
    install(FILES ${CMAKE_BINARY_DIR}/package.json DESTINATION .)
    install(FILES ${PROJECT_SOURCE_DIR}/../LICENSE DESTINATION .)
    install(TARGETS sfcompose DESTINATION .)
else()
    add_executable(sfsplit sfsplit.cpp  ${SOURCES})
    add_executable(sfcompose sfcompose.cpp  ${SOURCES})
endif()



