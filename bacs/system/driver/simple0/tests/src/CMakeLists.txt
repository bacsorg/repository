project(bunsan_bacs_system_driver_simple0_tests)

include(${CMAKE_SOURCE_DIR}/system-config.cmake OPTIONAL)

cmake_minimum_required(VERSION 2.8)

include(YandexContestCommon)

# BOOST
set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
find_package(Boost COMPONENTS system filesystem serialization iostreams unit_test_framework program_options thread REQUIRED)
list(APPEND includes ${Boost_INCLUDE_DIRS})
list(APPEND libraries ${Boost_LIBRARIES})

# Includes and libraries
include_directories(${includes})

# Targets

# Converter
add_executable(${PROJECT_NAME}
    src/main.cpp
)
target_link_libraries(${PROJECT_NAME} ${libraries})

# Tests

set(tests_src_dir ${PROJECT_SOURCE_DIR}/share/tests)

file(GLOB tests_src ${tests_src_dir}/*)

set(tests_dst_dir ${PROJECT_BINARY_DIR}/tests)
execute_process(COMMAND ${CMAKE_COMMAND} -E make_directory ${tests_dst_dir})

set(tests_dst)
foreach(test ${tests_src})
    get_filename_component(test_name ${test} NAME)
    list(APPEND tests_dst ${tests_dst_dir}/${test_name})
endforeach()
list(LENGTH tests_dst tests_dst_size)
if(tests_dst_size EQUAL 0)
    message(SEND_ERROR "No tests found at ${tests_src_dir}.")
endif()

add_custom_command(OUTPUT ${tests_dst}
    DEPENDS ${tests}
    COMMAND ${PROJECT_NAME} ${tests_dst_dir} ${tests_src}
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
add_custom_target(tests ALL DEPENDS ${tests_dst})

# Installation
install(FILES ${tests_dst} DESTINATION share/tests)
