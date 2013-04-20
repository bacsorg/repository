project(bunsan_bacs_utility_single_pas)

cmake_minimum_required(VERSION 2.8)

file(GLOB_RECURSE modules ${CMAKE_SOURCE_DIR}/modules/*.cmake)
foreach(module ${modules})
    message(AUTHOR_WARNING "Using ${module} module")
    include(${module})
endforeach()

set(fpc_libs)
foreach(lib ${libraries})
    set(lib_path ${PROJECT_SOURCE_DIR}/lib/${lib})
    list(APPEND fpc_libs -Fu${lib_path} -Fi${lib_path})
endforeach()
add_custom_command(OUTPUT ${PROJECT_BINARY_DIR}/${target}
    COMMAND fpc -TLINUX -FU${PROJECT_BINARY_DIR} ${fpc_libs} -Mdelphi -O2 ${PROJECT_SOURCE_DIR}/src/${source} -o${PROJECT_BINARY_DIR}/${target})
add_custom_target(${target}_target ALL
    DEPENDS ${PROJECT_BINARY_DIR}/${target})

install(PROGRAMS ${PROJECT_BINARY_DIR}/${target} DESTINATION bin)
