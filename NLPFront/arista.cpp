#include "arista.h"

Arista::Arista()
    : idOrigen()
    , idDestino()
    , peso(1.0)
    , sumaSentimiento(0.0)
    , cantidadSentimientos(0)
{
}

Arista::Arista(const std::string &idOrigen, const std::string &idDestino, double peso)
    : idOrigen(idOrigen)
    , idDestino(idDestino)
    , peso(peso)
    , sumaSentimiento(0.0)
    , cantidadSentimientos(0)
{
}

std::string Arista::obtenerIdOrigen() const
{
    return idOrigen;
}

void Arista::fijarIdOrigen(const std::string &idOrigen)
{
    this->idOrigen = idOrigen;
}

std::string Arista::obtenerIdDestino() const
{
    return idDestino;
}

void Arista::fijarIdDestino(const std::string &idDestino)
{
    this->idDestino = idDestino;
}

double Arista::obtenerPeso() const
{
    return peso;
}

void Arista::fijarPeso(double peso)
{
    this->peso = peso;
}

double Arista::obtenerSentimiento() const
{
    if (cantidadSentimientos == 0)
    {
        return 0.0;
    }

    return sumaSentimiento / cantidadSentimientos;
}

void Arista::agregarSentimiento(double puntaje)
{
    sumaSentimiento += puntaje;
    ++cantidadSentimientos;
}

bool Arista::operator==(const Arista &otra) const
{
    return idOrigen == otra.idOrigen && idDestino == otra.idDestino && peso == otra.peso;
}

bool Arista::operator!=(const Arista &otra) const
{
    return !(*this == otra);
}
