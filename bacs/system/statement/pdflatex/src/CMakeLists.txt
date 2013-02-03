project(statement_pdflatex)

cmake_minimum_required(VERSION 2.8)

find_package(LATEX)

include(source.cmake)

add_custom_command(
    OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/src/${target}
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/${source}
    COMMAND ${PDFLATEX_COMPILER} ${source}
    COMMAND ${PDFLATEX_COMPILER} ${source}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/src
)

add_custom_target(target ALL DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/${target})

install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/${target} DESTINATION data)
