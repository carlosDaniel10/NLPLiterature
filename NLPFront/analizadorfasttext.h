#ifndef ANALIZADORFASTTEXT_H
#define ANALIZADORFASTTEXT_H

#include "analizadorsentimientos.h"

#include <memory>
#include <string>

// Analizador de sentimiento basado en un modelo supervisado de fastText
// (Meta). Carga un modelo .bin entrenado con las etiquetas
// __label__positivo, __label__negativo y __label__neutral, y convierte las
// probabilidades de clasificacion en un puntaje en [-1, 1].
class AnalizadorFastText : public AnalizadorSentimientos
{
public:
    AnalizadorFastText();
    ~AnalizadorFastText();

    AnalizadorFastText(const AnalizadorFastText &otro) = delete;
    AnalizadorFastText &operator=(const AnalizadorFastText &otro) = delete;

    // Carga el modelo .bin. Devuelve false si no se pudo cargar.
    bool cargarModelo(const std::string &ruta);

    // True si hay un modelo cargado y listo para analizar.
    bool estaDisponible() const;

    // Analiza un texto y devuelve un puntaje en [-1, 1].
    double analizar(const std::string &texto) override;

private:
    // Implementacion oculta para no exponer las cabeceras de fastText al
    // resto del proyecto ni a moc.
    class Implementacion;
    std::unique_ptr<Implementacion> implementacion;
};

#endif // ANALIZADORFASTTEXT_H
