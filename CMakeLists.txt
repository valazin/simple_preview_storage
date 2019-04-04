cmake_minimum_required(VERSION 3.5.1)
project(simplepreviewstorage)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pedantic-errors")

set(APP_NAME "simplepreviewstorage")

###Sources
set(SRC_DIRS
    "src"
    "src/utility"
    "src/http"
    )

set(SRC "")
foreach(_src_dir ${SRC_DIRS})
    file(GLOB
        _files
        "${_src_dir}/*.h"
        "${_src_dir}/*.hpp"
        "${_src_dir}/*.cpp"
    )
    list(APPEND SRC ${_files})
endforeach()

add_executable(${APP_NAME} ${SRC})

#TODO:
target_include_directories(${APP_NAME} PRIVATE ${SRC_DIRS})

###3rd party libraries
set(LIBRARIES
    "pthread"
    "opencv_core"
    "opencv_imgcodecs"
    )
target_link_libraries(${APP_NAME} ${LIBRARIES})

###Install
install(TARGETS ${APP_NAME} DESTINATION bin)
