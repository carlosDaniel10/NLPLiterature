#ifndef DATO_H
#define DATO_H

#include <string>

// Dato que se almacena en un nodo del grafo.
class Dato
{
public:
    Dato();
    explicit Dato(const std::string &valor);

    std::string obtenerValor() const;
    void fijarValor(const std::string &valor);

    bool operator==(const Dato &otro) const;
    bool operator!=(const Dato &otro) const;

private:
    std::string valor;
};

#endif // DATO_H
