cmake_minimum_required(VERSION 3.11)

SET(SF_VERSION_MAJOR 1)
SET(SF_VERSION_MINOR 0)
SET(SF_VERSION_PATCH 0)

execute_process(COMMAND git --work-tree=. log --pretty=format:%h -n 1
              OUTPUT_VARIABLE GIT_VERSION)

if (${EMSCRIPTEN})
    MESSAGE("using emscripten")
    SET(USE_EMSCRIPTEN 1)
    configure_file("package.in.json" "${CMAKE_BINARY_DIR}/package.json")
endif()

project(sfcomposer)
include_directories(src)
add_subdirectory(src)

include(InstallRequiredSystemLibraries)
set(CPACK_PACKAGE_VENDOR "Samba Godschynski")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/../LICENSE")
set(CPACK_PACKAGE_VERSION_MAJOR "${SF_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${SF_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${SF_VERSION_PATCH}")

include(CPack)