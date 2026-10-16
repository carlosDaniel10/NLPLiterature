cmake_minimum_required(VERSION 3.16)

# Herramienta auxiliar (fuera del build de la aplicacion): entrena el modelo
# de sentimiento fastText a partir de las obras con etiquetado debil.
project(EntrenarModeloFastText VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core)
find_package(Threads REQUIRED)

# fastText (Meta): misma version, fuente y parche que usa la aplicacion.
include(FetchContent)
FetchContent_Declare(fasttext
        GIT_REPOSITORY https://github.com/facebookresearch/fastText.git
        GIT_TAG v0.9.2
        GIT_SHALLOW TRUE
        PATCH_COMMAND ${CMAKE_COMMAND}
            -DARCHIVO_ARGS=<SOURCE_DIR>/src/args.h
            -DARCHIVO_MAIN=<SOURCE_DIR>/src/main.cc
            -P ${CMAKE_CURRENT_SOURCE_DIR}/../../cmake/parchear_fasttext.cmake
)
FetchContent_MakeAvailable(fasttext)

set_target_properties(fasttext-shared fasttext-static_pic fasttext-bin PROPERTIES EXCLUDE_FROM_ALL TRUE)

# Reusa el analizador de lexico y el entrenador de la aplicacion.
add_executable(entrenarmodelo
        entrenarmodelo.cpp
        ../../analizadorlexico.cpp
        ../../entrenadorfasttext.cpp
)

target_include_directories(entrenarmodelo PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../..
        ${fasttext_SOURCE_DIR}/src
)

target_link_libraries(entrenarmodelo PRIVATE Qt6::Core fasttext-static Threads::Threads)
