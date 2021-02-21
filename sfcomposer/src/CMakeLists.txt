set (SOURCES 
    main.cpp 
    sf3/myfile.cpp
    sf3/mysysinfo.cpp
    sf3/mystring.cpp
    sf3/sfont.cpp
)

add_executable(sfcomposer ${SOURCES})
target_link_libraries(sfcomposer ogg vorbis vorbisfile vorbisenc)