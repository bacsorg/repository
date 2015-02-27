project(bunsan_bacs_lib_pas)

cmake_minimum_required(VERSION 3.0)

file(GLOB sources src/*.pas src/*.dpr)

set(src_dir ${PROJECT_SOURCE_DIR}/src)

set(binaries)

foreach(src ${sources})
    get_filename_component(trgt ${src} NAME_WE)
    set(depends ${PROJECT_BINARY_DIR}/${trgt}.o ${PROJECT_BINARY_DIR}/${trgt}.ppu)
    list(APPEND binaries ${depends})
    add_custom_command(OUTPUT ${depends}
        COMMAND fpc -Mdelphi -FU${PROJECT_BINARY_DIR} -Fo${PROJECT_BINARY_DIR} -Fi${src_dir} -Fu${PROJECT_BINARY_DIR} ${src}
        DEPENDS ${src})
endforeach()
add_custom_target(pascal ALL
    DEPENDS ${binaries})

file(GLOB includes src/*.inc)

file(GLOB_RECURSE modules ${CMAKE_SOURCE_DIR}/modules/*.cmake)
foreach(module ${modules})
    message(AUTHOR_WARNING "Using ${module} module")
    include(${module})
endforeach()
