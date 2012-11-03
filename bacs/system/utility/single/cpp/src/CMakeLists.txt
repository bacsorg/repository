project(bunsan_bacs_utility_single_cpp)

cmake_minimum_required(VERSION 2.8)

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
