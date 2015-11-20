cmake_minimum_required(VERSION 3.0)

project(bacs_system_single_test_storage)

find_package(BacsCommon CONFIG REQUIRED)
bacs_worker_project()

bunsan_add_module_library(${PROJECT_NAME}
    src/copy.cpp
)
# FIXME no linkage requirement
bunsan_use_bunsan_package(${PROJECT_NAME} bacs_system_single
    bacs_system_single
)

bunsan_install_targets(${PROJECT_NAME})
bunsan_install_project()
