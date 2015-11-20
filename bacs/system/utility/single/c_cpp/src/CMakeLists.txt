cmake_minimum_required(VERSION 3.0)

project(bacs_utility_single_cpp)

find_package(BacsCommon CONFIG REQUIRED)
bacs_worker_project()

file(GLOB_RECURSE modules ${CMAKE_SOURCE_DIR}/modules/*.cmake)
foreach(module ${modules})
    message(AUTHOR_WARNING "Using ${module} module")
    include(${module})
endforeach()

add_executable(${target} src/${source})
foreach(lib ${libraries})
    include_directories(lib/${lib})
endforeach()

install(TARGETS ${target}
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib)
