#include "nodo.h"

Nodo::Nodo()
    : id()
    , dato()
{
}

Nodo::Nodo(const std::string &id, const Dato &dato)
    : id(id)
    , dato(dato)
{
}

std::string Nodo::obtenerId() const
{
    return id;
}

void Nodo::fijarId(const std::string &id)
{
    this->id = id;
}

Dato Nodo::obtenerDato() const
{
    return dato;
}

void Nodo::fijarDato(const Dato &dato)
{
    this->dato = dato;
}

bool Nodo::operator==(const Nodo &otro) const
{
    return id == otro.id;
}

bool Nodo::operator!=(const Nodo &otro) const
{
    return !(*this == otro);
}
