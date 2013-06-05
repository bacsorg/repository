project(bacs_system_single)

include(${CMAKE_SOURCE_DIR}/system-config.cmake OPTIONAL)

cmake_minimum_required(VERSION 2.8)

include(YandexContestCommon)

bunsan_use_boost(system filesystem serialization iostreams unit_test_framework program_options regex thread)
bunsan_use_xmlrpc(c++2 libwww-client abyss-server)
bunsan_use_protobuf()
bunsan_use_bunsan(common pm bacs_problem bacs_problem_single)
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
    src/lib/builder.cpp
    src/lib/testing.cpp
    src/lib/testing_test.cpp
    src/lib/callback.cpp

    src/lib/detail/process.cpp
    src/lib/detail/file.cpp
    src/lib/detail/result.cpp

    src/lib/callbacks/xmlrpc.cpp

    src/lib/builders/compilable.cpp
    src/lib/builders/native_compilable.cpp
    src/lib/builders/gcc.cpp
    src/lib/builders/fpc.cpp
    src/lib/builders/interpretable.cpp
    src/lib/builders/python.cpp
    src/lib/builders/java.cpp
    src/lib/builders/mono.cpp
)

bunsan_install_headers()
bunsan_install_targets(${PROJECT_NAME})

#bunsan_include_tests()
