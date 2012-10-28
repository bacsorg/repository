project(bacs_single)

include(${CMAKE_SOURCE_DIR}/system-config.cmake OPTIONAL)

cmake_minimum_required(VERSION 2.8)

include(YandexContestCommon)

# BOOST
set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
find_package(Boost COMPONENTS system filesystem serialization iostreams unit_test_framework program_options regex thread REQUIRED)
list(APPEND includes ${Boost_INCLUDE_DIRS})
list(APPEND libraries ${Boost_LIBRARIES})

# XML-RPC
find_package(XMLRPC REQUIRED c++2 libwww-client abyss-server)
list(APPEND includes ${XMLRPC_INCLUDE_DIRS})
list(APPEND libraries ${XMLRPC_LIBRARIES})

# Local
include_directories(include ${includes})

# ProtoBuf
find_package(Protobuf REQUIRED)
list(APPEND includes ${PROTOBUF_INCLUDE_DIRS})
list(APPEND libraries ${PROTOBUF_LIBRARIES})

# bunsan
list(APPEND libraries bunsan_common bunsan_pm)

# bacs::single::api
list(APPEND libraries bunsan_bacs_single_api)

# yandex_contest_(
list(APPEND libraries
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
add_executable(${PROJECT_NAME}
    ${sources}
    src/bin/main.cpp

    src/lib/tests.cpp
    src/lib/process.cpp
    src/lib/testing.cpp
    src/lib/testing_test.cpp
    src/lib/callback.cpp

    src/lib/detail/file.cpp

    src/lib/callbacks/xmlrpc.cpp
)
target_link_libraries(${PROJECT_NAME} ${libraries})

# Installation
install(DIRECTORY include DESTINATION .)
install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib)

#yandex_contest_tests()
