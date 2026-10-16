# Parche para fastText v0.9.2: dos ajustes para compilar y enlazar con
# GCC 13 en este proyecto.
#
# 1. Agrega #include <cstdint> en src/args.h: sin ese include el codigo no
#    compila con GCC 13 (libstdc++ ya no lo incluye de forma transitiva).
# 2. Renombra el main del ejecutable CLI (src/main.cc) a fasttext_cli_main:
#    fastText compila su CLI dentro de la libreria estatica y ese main se
#    interpondria con el de la aplicacion al enlazar.

if(NOT DEFINED ARCHIVO_ARGS)
    message(FATAL_ERROR "ARCHIVO_ARGS no definido")
endif()

if(NOT DEFINED ARCHIVO_MAIN)
    message(FATAL_ERROR "ARCHIVO_MAIN no definido")
endif()

file(READ "${ARCHIVO_ARGS}" CONTENIDO)

string(FIND "${CONTENIDO}" "#include <cstdint>" EXISTENTE)

if(EXISTENTE EQUAL -1)
    string(FIND "${CONTENIDO}" "#include <vector>" POSICION)

    if(POSICION EQUAL -1)
        message(FATAL_ERROR "No se encontro '#include <vector>' en args.h")
    endif()

    string(REPLACE "#include <vector>" "#include <cstdint>\n#include <vector>" CONTENIDO "${CONTENIDO}")
    file(WRITE "${ARCHIVO_ARGS}" "${CONTENIDO}")
endif()

file(READ "${ARCHIVO_MAIN}" CONTENIDO_MAIN)

string(FIND "${CONTENIDO_MAIN}" "fasttext_cli_main" YA_PARCEADO)

if(YA_PARCEADO EQUAL -1)
    string(FIND "${CONTENIDO_MAIN}" "int main(int argc, char** argv)" POSICION_MAIN)

    if(POSICION_MAIN EQUAL -1)
        message(FATAL_ERROR "No se encontro 'int main(int argc, char** argv)' en main.cc")
    endif()

    string(REPLACE "int main(int argc, char** argv)" "int fasttext_cli_main(int argc, char** argv)" CONTENIDO_MAIN "${CONTENIDO_MAIN}")
    file(WRITE "${ARCHIVO_MAIN}" "${CONTENIDO_MAIN}")
endif()
