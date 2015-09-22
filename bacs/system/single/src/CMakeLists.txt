cmake_minimum_required(VERSION 3.0)

project(bacs_system_single_worker)

if(BunsanCMakeBuiltin)
    BunsanCMakeLoad()
else()
    find_package(BunsanCMake CONFIG REQUIRED)
endif()

bunsan_add_executable(${PROJECT_NAME}
    src/bin/main.cpp
)
bunsan_use_bunsan_package(${PROJECT_NAME} bacs_system_single
    bacs_system_single
)

#bunsan_install_headers()
bunsan_install_targets(${PROJECT_NAME})
#bunsan_install_project()

#bunsan_include_tests()
