project(bacs_system_single)

include(${CMAKE_SOURCE_DIR}/system-config.cmake OPTIONAL)

cmake_minimum_required(VERSION 3.0)

find_package(BunsanCMake CONFIG REQUIRED)

file(GLOB_RECURSE modules ${CMAKE_SOURCE_DIR}/modules/*.cmake)
foreach(module ${modules})
    message(AUTHOR_WARNING "Using ${module} module")
    include(${module})
endforeach()

# Targets
message(AUTHOR_WARNING "Building ${PROJECT_NAME}")
message(AUTHOR_WARNING "Additional sources: ${bacs_sources}")
message(AUTHOR_WARNING "Additional includes: ${bacs_include_dirs}")
message(AUTHOR_WARNING "Additional libraries: ${bacs_libraries}")
bunsan_add_executable(${PROJECT_NAME}
    ${bacs_sources}

    src/bin/main.cpp

    src/lib/detail/file.cpp
    src/lib/testing.cpp
    src/lib/tests.cpp
)
bunsan_use_bunsan_package(${PROJECT_NAME} bunsan_bacs_system
    bunsan_bacs_system
)
bunsan_use_bunsan_package(${PROJECT_NAME} bunsan_bacs_problem_single
    bunsan_bacs_problem_single
)
bunsan_use_bunsan_package(${PROJECT_NAME} yandex_contest_invoker yandex_contest_invoker)
bunsan_use_bunsan_package(${PROJECT_NAME} bunsan_broker bunsan_broker)
bunsan_use_bunsan_package(${PROJECT_NAME} bunsan_protobuf bunsan_protobuf)
bunsan_use_boost(${PROJECT_NAME} regex)
target_include_directories(${PROJECT_NAME} PRIVATE ${bacs_include_dirs})
target_link_libraries(${PROJECT_NAME} PRIVATE ${bacs_libraries})

bunsan_install_headers()
bunsan_install_targets(${PROJECT_NAME})
#bunsan_install_project()

#bunsan_include_tests()
