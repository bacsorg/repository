project(statement_pdflatex)

cmake_minimum_required(VERSION 3.0)

find_package(LATEX)

include(source.cmake)

add_custom_command(
    OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/src/${root}/${target}
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/${root}/${source}
    COMMAND ${PDFLATEX_COMPILER} ${source}
    COMMAND ${PDFLATEX_COMPILER} ${source}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/src/${root}
)

add_custom_target(target ALL DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/${root}/${target})

install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/${root}/${target} DESTINATION data)
