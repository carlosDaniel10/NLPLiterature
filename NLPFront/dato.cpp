#include "dato.h"

Dato::Dato()
    : valor()
{
}

Dato::Dato(const std::string &valor)
    : valor(valor)
{
}

std::string Dato::obtenerValor() const
{
    return valor;
}

void Dato::fijarValor(const std::string &valor)
{
    this->valor = valor;
}

bool Dato::operator==(const Dato &otro) const
{
    return valor == otro.valor;
}

bool Dato::operator!=(const Dato &otro) const
{
    return !(*this == otro);
}
