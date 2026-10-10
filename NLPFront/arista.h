#ifndef ARISTA_H
#define ARISTA_H

#include <string>

// Arista entre dos nodos identificados por su id.
class Arista
{
public:
    Arista();
    Arista(const std::string &idOrigen, const std::string &idDestino, double peso = 1.0);

    std::string obtenerIdOrigen() const;
    void fijarIdOrigen(const std::string &idOrigen);

    std::string obtenerIdDestino() const;
    void fijarIdDestino(const std::string &idDestino);

    double obtenerPeso() const;
    void fijarPeso(double peso);

    bool operator==(const Arista &otra) const;
    bool operator!=(const Arista &otra) const;

private:
    std::string idOrigen;
    std::string idDestino;
    double peso;
};

#endif // ARISTA_H
