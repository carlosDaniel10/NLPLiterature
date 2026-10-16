#ifndef ENTRENADORFASTTEXT_H
#define ENTRENADORFASTTEXT_H

#include <functional>
#include <string>

// Entrena y cuantiza el modelo de sentimiento fastText. La logica se comparte
// entre la herramienta de consola (herramientas/entrenarmodelo) y el dialogo
// de reentrenamiento de la aplicacion.
class EntrenadorFastText
{
public:
    using Reportador = std::function<void(const std::string &linea)>;
    using ReporteProgreso = std::function<void(int porcentaje)>;

    EntrenadorFastText();

    // Etapa 1 (modo completo): lee las obras XML-TEI de carpetaObras, las
    // segmenta, las etiqueta debilmente con el texto del lexico de polaridad
    // y escribe el archivo de entrenamiento en rutaTrain. Devuelve la
    // cantidad de muestras escritas o -1 en caso de error.
    long generarArchivoEntrenamiento(const std::string &carpetaObras,
                                     const std::string &contenidoLexico,
                                     const std::string &rutaTrain,
                                     const Reportador &reportar,
                                     const ReporteProgreso &reportarProgreso);

    // Etapas 2-4: entrena con rutaTrain, cuantiza y guarda salida + ".ftz";
    // elimina salida + ".bin" y salida + ".vec" si quedaron de corridas
    // anteriores. Devuelve false en caso de error.
    bool entrenar(const std::string &rutaTrain,
                  const std::string &salida,
                  int epochs,
                  int dim,
                  int bucket,
                  const Reportador &reportar,
                  const ReporteProgreso &reportarProgreso);
};

#endif // ENTRENADORFASTTEXT_H
