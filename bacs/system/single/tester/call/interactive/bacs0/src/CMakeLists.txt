cmake_minimum_required(VERSION 3.0)

project(bacs_system_single_tester)

if(BunsanCMakeBuiltin)
    BunsanCMakeLoad()
else()
    find_package(BunsanCMake CONFIG REQUIRED)
endif()

bunsan_add_module_library(${PROJECT_NAME}
    src/interactive_bacs0.cpp
)
# FIXME no linkage requirement
bunsan_use_bunsan_package(${PROJECT_NAME} bacs_system_single
    bacs_system_single
)

bunsan_install_targets(${PROJECT_NAME})
bunsan_install_project()
