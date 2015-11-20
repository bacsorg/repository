cmake_minimum_required(VERSION 3.0)

project(bacs_system_single_list_tests_builder)

find_package(BacsCommon CONFIG REQUIRED)
bacs_worker_project()

bunsan_add_executable(${PROJECT_NAME}
    src/main.cpp
)
bunsan_use_bunsan_package(${PROJECT_NAME} bunsan_common bunsan_common)

bunsan_install_targets(${PROJECT_NAME})
install(FILES TestsCMakeLists.txt DESTINATION . RENAME CMakeLists.txt)
