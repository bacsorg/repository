cmake_minimum_required(VERSION 3.0)

project(bacs_system_system_verifier)

find_package(BacsCommon CONFIG REQUIRED)
bacs_worker_project()

bunsan_add_module_library(${PROJECT_NAME}
    src/system_verifier.cpp
)
# FIXME no linkage requirement
bunsan_use_bunsan_package(${PROJECT_NAME} bunsan_protobuf
    bunsan_protobuf
)
bunsan_use_bunsan_package(${PROJECT_NAME} bacs_system
    bacs_system
)

bunsan_install_targets(${PROJECT_NAME})
bunsan_install_project()
