cmake_minimum_required(VERSION 3.0)

project(bacs_system_system_verifier)

if(BunsanCMakeBuiltin)
    BunsanCMakeLoad()
else()
    find_package(BunsanCMake CONFIG REQUIRED)
endif()

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
