cmake_minimum_required(VERSION 3.11)

execute_process(COMMAND git --work-tree=. log --pretty=format:%h -n 1
              OUTPUT_VARIABLE GIT_VERSION)
# Project name.
project(sfcomposer)

add_subdirectory(src)


