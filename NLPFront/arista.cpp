#include "arista.h"

Arista::Arista()
    : idOrigen()
    , idDestino()
    , peso(1.0)
{
}

Arista::Arista(const std::string &idOrigen, const std::string &idDestino, double peso)
    : idOrigen(idOrigen)
    , idDestino(idDestino)
    , peso(peso)
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

bool Arista::operator==(const Arista &otra) const
{
    return idOrigen == otra.idOrigen && idDestino == otra.idDestino && peso == otra.peso;
}

bool Arista::operator!=(const Arista &otra) const
{
    return !(*this == otra);
}
