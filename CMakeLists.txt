cmake_minimum_required(VERSION 3.30)

project(PianoTutor
    LANGUAGES C
)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_FLAGS "-Wall -Wextra -Wshadow -O0 -ggdb")
# set(CMAKE_C_FLAGS "-Wall -Wextra -Wshadow -O3")

find_package(PkgConfig REQUIRED)

pkg_check_modules(CGLM REQUIRED cglm)
pkg_check_modules(GLEW REQUIRED glew)
pkg_check_modules(GLFW REQUIRED glfw3)
pkg_check_modules(LIBZIP REQUIRED libzip)
# pkg_check_modules(LIBXML2 REQUIRED libxml2)
find_package(LIBXML2 REQUIRED)

include_directories(
    ${CGLM_INCLUDE_DIRS}
    ${GLEF_INCLUDE_DIRS}
    ${GLFW_INCLUDE_DIRS}
    ${LIBZIP_INCLUDE_DIRS}
    ${LIBXML2_INCLUDE_DIRS}
)

link_directories(
    ${CGLM_LIBRARY_DIRS}
    ${GLEF_LIBRARY_DIRS}
    ${GLFW_LIBRARY_DIRS}
    ${LIBZIP_LIBRARY_DIRS}
    ${LIBXML2_LIBRARY_DIRS}
)

set(SOURCES
    ./src/main.c
    ./src/inputParser.c
    ./src/helpers.c

    ./src/xmlParser.c
    ./src/xmlAttributeParser.c
    ./src/xmlNoteParser.c
    ./src/compute.c
    ./src/computeFunc.c
    ./src/computeBeam.c
    
    ./src/graphics.c
    ./src/shader.c
    ./src/text.c

    ./src/piano.c
    ./src/pianoMeshes.c
    ./src/pianoFunc.c
    ./src/view.c
    ./src/pianoDraw.c

    ./src/midiDevice.c
)

add_executable(main ${SOURCES})

target_link_libraries(main
    ${CGLM_LIBRARIES}
    ${GLEW_LIBRARIES}
    ${GLFW_LIBRARIES}
    ${LIBZIP_LIBRARIES}
    ${LIBXML2_LIBRARIES}
    m
)
