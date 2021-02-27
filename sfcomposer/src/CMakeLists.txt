set (SOURCES 
    sf3/myfile.cpp
    sf3/mysysinfo.cpp
    sf3/mystring.cpp
    sf3/sfont.cpp
)

add_executable(sfsplit sfsplit.cpp  ${SOURCES})
add_executable(sfcompose sfcompose.cpp  ${SOURCES})