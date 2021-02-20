cmake_minimum_required(VERSION 3.11)

execute_process(COMMAND git --work-tree=. log --pretty=format:%h -n 1
              OUTPUT_VARIABLE GIT_VERSION)
# Project name.
project(sfcomposer)
add_definitions(-DM_PI=3.14159)
include_directories(ext/ogg/include)
include_directories(ext/vorbis/include)
add_subdirectory(src)
add_subdirectory(ext/ogg)
add_subdirectory(ext/vorbis)


