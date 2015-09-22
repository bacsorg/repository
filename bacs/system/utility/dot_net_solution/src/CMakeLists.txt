cmake_minimum_required(VERSION 3.0)

project(bacs_utility_dot_net_solution)

# Default values, may be overwritten in modules
set(configuration Release)

file(GLOB_RECURSE modules ${CMAKE_SOURCE_DIR}/modules/*.cmake)
foreach(module ${modules})
    message(AUTHOR_WARNING "Using ${module} module")
    include(${module})
endforeach()

set(dot_net_libs)
foreach(lib ${libraries})
    set(lib_path ${PROJECT_SOURCE_DIR}/lib/${lib})
    message(SEND_ERROR "Libraries are not supported")
    list(APPEND dot_net_libs ${lib_path})
endforeach()
add_custom_command(OUTPUT ${PROJECT_BINARY_DIR}/${target}/bin/${target}
    COMMAND bacs_system_build_mono_develop
            --fhs
            --output ${PROJECT_BINARY_DIR}/${target}
            --executable ${target}
            --application ${target}
            --configuration=${configuration}
            ${PROJECT_SOURCE_DIR}/src/${solution}
)
add_custom_target(${target}_target ALL
    DEPENDS ${PROJECT_BINARY_DIR}/${target}/bin/${target})

install(
    DIRECTORY ${PROJECT_BINARY_DIR}/${target}/
    DESTINATION .
    USE_SOURCE_PERMISSIONS
)
