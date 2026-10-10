#ifndef NODO_H
#define NODO_H

#include "dato.h"

#include <string>

// Nodo del grafo: un id unico y el dato que almacena.
class Nodo
{
public:
    Nodo();
    Nodo(const std::string &id, const Dato &dato);

    std::string obtenerId() const;
    void fijarId(const std::string &id);

    Dato obtenerDato() const;
    void fijarDato(const Dato &dato);

    bool operator==(const Nodo &otro) const;
    bool operator!=(const Nodo &otro) const;

private:
    std::string id;
    Dato dato;
};

#endif // NODO_H
