project(bacs_system_single)

include(${CMAKE_SOURCE_DIR}/system-config.cmake OPTIONAL)

cmake_minimum_required(VERSION 2.8)

include(BunsanCommon)

bunsan_use_boost(system filesystem regex serialization)
bunsan_use_protobuf()
bunsan_use_bunsan(
    common
    bacs_common
    bacs_system
    bacs_problem
    bacs_problem_single
)
bunsan_use(
    yandex_contest_common
    yandex_contest_system
    yandex_contest_invoker
)

file(GLOB_RECURSE modules ${CMAKE_SOURCE_DIR}/modules/*.cmake)
foreach(module ${modules})
    message(AUTHOR_WARNING "Using ${module} module")
    include(${module})
endforeach()

# Targets
message(AUTHOR_WARNING "Building ${PROJECT_NAME} using ${sources} linking with ${libraries}")
bunsan_add_executable(${PROJECT_NAME}
    ${sources}
    src/bin/main.cpp

    src/lib/tests.cpp
    src/lib/testing.cpp

    src/lib/detail/file.cpp
)

bunsan_install_headers()
bunsan_install_targets(${PROJECT_NAME})

#bunsan_include_tests()
