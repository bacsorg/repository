cmake_minimum_required(VERSION 3.0)

project(bacs_lib_cpp)

#file(GLOB sources src/*.cpp src/*.cc)
file(GLOB includes include/*.h include/*.hpp)

file(GLOB_RECURSE modules ${CMAKE_SOURCE_DIR}/modules/*.cmake)
foreach(module ${modules})
    message(AUTHOR_WARNING "Using ${module} module")
    include(${module})
endforeach()
