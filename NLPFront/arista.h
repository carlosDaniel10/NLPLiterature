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

    // Sentimiento promedio acumulado en [-1, 1]; 0.0 si aun no hay datos.
    double obtenerSentimiento() const;
    void agregarSentimiento(double puntaje);

    bool operator==(const Arista &otra) const;
    bool operator!=(const Arista &otra) const;

private:
    std::string idOrigen;
    std::string idDestino;
    double peso;
    double sumaSentimiento;
    int cantidadSentimientos;
};

#endif // ARISTA_H
