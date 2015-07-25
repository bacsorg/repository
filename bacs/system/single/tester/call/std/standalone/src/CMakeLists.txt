cmake_minimum_required(VERSION 3.0)

project(bacs_single_system_tester)

if(BunsanCMakeBuiltin)
    BunsanCMakeLoad()
else()
    find_package(BunsanCMake CONFIG REQUIRED)
endif()

bunsan_add_module_library(${PROJECT_NAME}
    src/standalone.cpp
)
# FIXME no linkage requirement
bunsan_use_bunsan_package(${PROJECT_NAME} bunsan_bacs_system_single
    bunsan_bacs_system_single
)

bunsan_install_targets(${PROJECT_NAME})
bunsan_install_project()
