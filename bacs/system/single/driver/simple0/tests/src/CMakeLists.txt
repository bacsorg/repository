project(bunsan_bacs_system_driver_simple0_tests_builder)

include(${CMAKE_SOURCE_DIR}/system-config.cmake OPTIONAL)

cmake_minimum_required(VERSION 2.8)

include(BunsanCommon)

bunsan_use_boost(system filesystem serialization program_options thread)
bunsan_use_bunsan(common)

bunsan_add_executable(${PROJECT_NAME}
    src/main.cpp
)

bunsan_install_targets(${PROJECT_NAME})
install(FILES TestsCMakeLists.txt DESTINATION . RENAME CMakeLists.txt )
