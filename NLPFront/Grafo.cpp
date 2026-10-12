#include "grafo.h"

#include <iostream>
#include <stdexcept>

Grafo::Grafo(bool direccionado, bool ponderado)
    : nodos()
    , aristas()
    , direccionado(direccionado)
    , ponderado(ponderado)
{
}

bool Grafo::agregarNodo(const Nodo &nodo)
{
    if (existeNodo(nodo.obtenerId()))
    {
        return false;
    }

    nodos.emplace(nodo.obtenerId(), nodo);
    aristas.emplace(nodo.obtenerId(), std::vector<Arista>());
    return true;
}

bool Grafo::agregarArista(const Arista &arista)
{
    const std::string idOrigen = arista.obtenerIdOrigen();
    const std::string idDestino = arista.obtenerIdDestino();

    if (!existeNodo(idOrigen) || !existeNodo(idDestino))
    {
        return false;
    }

    if (existeArista(idOrigen, idDestino))
    {
        return false;
    }

    Arista nuevaArista = arista;

    if (!ponderado)
    {
        nuevaArista.fijarPeso(1.0);
    }

    aristas.at(idOrigen).push_back(nuevaArista);

    if (!direccionado && idOrigen != idDestino)
    {
        const Arista aristaInversa(idDestino, idOrigen, nuevaArista.obtenerPeso());
        aristas.at(idDestino).push_back(aristaInversa);
    }

    return true;
}

bool Grafo::existeNodo(const std::string &id) const
{
    return nodos.find(id) != nodos.end();
}

bool Grafo::existeArista(const std::string &idOrigen, const std::string &idDestino) const
{
    const auto it = aristas.find(idOrigen);

    if (it == aristas.end())
    {
        return false;
    }

    for (const Arista &arista : it->second)
    {
        if (arista.obtenerIdDestino() == idDestino)
        {
            return true;
        }
    }

    return false;
}

Nodo Grafo::obtenerNodo(const std::string &id) const
{
    const auto it = nodos.find(id);

    if (it == nodos.end())
    {
        throw std::out_of_range("El nodo no existe en el grafo.");
    }

    return it->second;
}

double Grafo::obtenerPeso(const std::string &idOrigen, const std::string &idDestino) const
{
    const auto it = aristas.find(idOrigen);

    if (it != aristas.end())
    {
        for (const Arista &arista : it->second)
        {
            if (arista.obtenerIdDestino() == idDestino)
            {
                return arista.obtenerPeso();
            }
        }
    }

    throw std::out_of_range("La arista no existe en el grafo.");
}

std::vector<std::string> Grafo::obtenerVecinos(const std::string &id) const
{
    const auto it = aristas.find(id);

    if (it == aristas.end())
    {
        throw std::out_of_range("El nodo no existe en el grafo.");
    }

    std::vector<std::string> vecinos;

    for (const Arista &arista : it->second)
    {
        vecinos.push_back(arista.obtenerIdDestino());
    }

    return vecinos;
}

std::vector<Arista> Grafo::obtenerAristas() const
{
    std::vector<Arista> resultado;

    for (const auto &par : aristas)
    {
        for (const Arista &arista : par.second)
        {
            if (direccionado || arista.obtenerIdOrigen() <= arista.obtenerIdDestino())
            {
                resultado.push_back(arista);
            }
        }
    }

    return resultado;
}

std::vector<Nodo> Grafo::obtenerNodos() const
{
    std::vector<Nodo> resultado;

    for (const auto &par : nodos)
    {
        resultado.push_back(par.second);
    }

    return resultado;
}

bool Grafo::fijarPeso(const std::string &idOrigen, const std::string &idDestino, double peso)
{
    if (!existeArista(idOrigen, idDestino))
    {
        return false;
    }

    const double pesoFinal = ponderado ? peso : 1.0;

    for (Arista &arista : aristas.at(idOrigen))
    {
        if (arista.obtenerIdDestino() == idDestino)
        {
            arista.fijarPeso(pesoFinal);
        }
    }

    if (!direccionado && idOrigen != idDestino)
    {
        for (Arista &arista : aristas.at(idDestino))
        {
            if (arista.obtenerIdDestino() == idOrigen)
            {
                arista.fijarPeso(pesoFinal);
            }
        }
    }

    return true;
}

bool Grafo::aumentarPeso(const std::string &idOrigen, const std::string &idDestino, double incremento)
{
    if (!existeNodo(idOrigen) || !existeNodo(idDestino))
    {
        return false;
    }

    if (existeArista(idOrigen, idDestino))
    {
        const double pesoActual = obtenerPeso(idOrigen, idDestino);
        return fijarPeso(idOrigen, idDestino, pesoActual + incremento);
    }

    return agregarArista(Arista(idOrigen, idDestino, incremento));
}

bool Grafo::esDireccionado() const
{
    return direccionado;
}

bool Grafo::esPonderado() const
{
    return ponderado;
}

std::size_t Grafo::cantidadNodos() const
{
    return nodos.size();
}

std::size_t Grafo::cantidadAristas() const
{
    return obtenerAristas().size();
}

void Grafo::limpiar()
{
    nodos.clear();
    aristas.clear();
}

void Grafo::imprimir() const
{
    std::cout << "Grafo direccionado: " << (direccionado ? "si" : "no")
              << ", ponderado: " << (ponderado ? "si" : "no") << "\n";

    for (const auto &par : nodos)
    {
        std::cout << "Nodo " << par.first << " (" << par.second.obtenerDato().obtenerValor() << "):";

        const auto it = aristas.find(par.first);

        if (it != aristas.end())
        {
            for (const Arista &arista : it->second)
            {
                std::cout << " -> " << arista.obtenerIdDestino()
                          << " (peso " << arista.obtenerPeso() << ")";
            }
        }

        std::cout << "\n";
    }
}
