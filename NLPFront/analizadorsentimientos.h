#ifndef ANALIZADORSENTIMIENTOS_H
#define ANALIZADORSENTIMIENTOS_H

#include <string>

// Interfaz comun de los analizadores de sentimiento: devuelven un puntaje en
// [-1, 1], positivo para sentimientos positivos y negativo para negativos.
class AnalizadorSentimientos
{
public:
    virtual ~AnalizadorSentimientos() = default;

    // Analiza un texto y devuelve un puntaje en [-1, 1].
    // No es const porque fastText::predict no lo es.
    virtual double analizar(const std::string &texto) = 0;
};

#endif // ANALIZADORSENTIMIENTOS_H
