project(bunsan_bacs_utility_single_java)

cmake_minimum_required(VERSION 2.8)

find_package(Java REQUIRED)
include(UseJava)

if(UNIX)
    set(platform unix)
else()
    message(SEND_ERROR "Platform is not supported!")
endif()

set(java_source_version "1.7")
set(java_target_version "1.7")

file(GLOB_RECURSE modules ${CMAKE_SOURCE_DIR}/modules/*.cmake)
foreach(module ${modules})
    message(AUTHOR_WARNING "Using ${module} module")
    include(${module})
endforeach()

set(target_jar ${target}_jar)

set(CMAKE_JAVA_COMPILE_FLAGS
    "-source" ${java_source_version}
    "-target" ${java_target_version}
)

get_filename_component(source_class ${source} NAME_WE)

add_jar(${target_jar}
    OUTPUT_NAME ${target}
    SOURCES src/${source}
    ENTRY_POINT ${source_class}
)

get_target_property(target_jar_path ${target_jar} JAR_FILE)
get_filename_component(target_jar_name ${target_jar_path} NAME)

configure_file(wrap/${platform}/main.cpp.in ${CMAKE_BINARY_DIR}/wrap.cpp
    ESCAPE_QUOTES
)
add_executable(${target} ${CMAKE_BINARY_DIR}/wrap.cpp)

install(FILES ${target_jar_path} DESTINATION lib)
install(TARGETS ${target} DESTINATION bin)
