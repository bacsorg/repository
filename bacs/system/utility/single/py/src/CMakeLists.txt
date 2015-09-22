cmake_minimum_required(VERSION 3.0)

project(bacs_utility_single_cpp)

file(GLOB_RECURSE modules ${CMAKE_SOURCE_DIR}/modules/*.cmake)
foreach(module ${modules})
    message(AUTHOR_WARNING "Using ${module} module")
    include(${module})
endforeach()

install(
    PROGRAMS src/${source}
    RENAME ${target}
    DESTINATION bin
)
