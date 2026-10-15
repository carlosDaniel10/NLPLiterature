#ifndef ANALIZADORSENTIMIENTOS_H
#define ANALIZADORSENTIMIENTOS_H

#include <cstddef>
#include <string>
#include <unordered_map>

// Analizador de sentimiento basado en un lexico de polaridad: devuelve un
// puntaje en [-1, 1], positivo para sentimientos positivos y negativo para
// sentimientos negativos. No usa librerias externas.
class AnalizadorSentimientos
{
public:
    AnalizadorSentimientos();

    // Carga el lexico desde un archivo de texto con lineas "palabra puntaje".
    // Devuelve false si el archivo no se pudo abrir.
    bool cargarLexico(const std::string &ruta);

    // Carga el lexico desde el contenido de un archivo ya leido.
    void cargarLexicoDesdeTexto(const std::string &contenido);

    // Analiza un texto y devuelve un puntaje en [-1, 1].
    double analizar(const std::string &texto) const;

    // Cantidad de palabras cargadas del lexico.
    std::size_t cantidadPalabras() const;

private:
    void procesarPalabra(const std::string &palabra, int &ventanaNegacion, double &suma, int &totalPalabras) const;
    bool esNegacion(const std::string &palabra) const;

    std::unordered_map<std::string, double> lexico;
};

#endif // ANALIZADORSENTIMIENTOS_H
