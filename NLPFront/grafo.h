#ifndef GRAFO_H
#define GRAFO_H

#include "arista.h"
#include "nodo.h"

#include <cstddef>
#include <string>
#include <unordered_map>
#include <vector>

// Grafo que guarda la lista de aristas en un unordered_map: la clave es el
// id del nodo origen y el valor es la lista de aristas que salen de ese nodo.
// Si el grafo no es direccionado, cada arista se guarda en ambos sentidos.
// Si el grafo no es ponderado, todos los pesos se normalizan a 1.0.
class Grafo
{
public:
    Grafo(bool direccionado = false, bool ponderado = false);

    // Devuelve false si ya existe un nodo con ese id.
    bool agregarNodo(const Nodo &nodo);

    // Devuelve false si algun nodo no existe o si la arista ya existe.
    bool agregarArista(const Arista &arista);

    bool existeNodo(const std::string &id) const;
    bool existeArista(const std::string &idOrigen, const std::string &idDestino) const;

    // Lanza std::out_of_range si el nodo no existe.
    Nodo obtenerNodo(const std::string &id) const;

    // Lanza std::out_of_range si la arista no existe.
    double obtenerPeso(const std::string &idOrigen, const std::string &idDestino) const;

    // Lanza std::out_of_range si el nodo no existe.
    std::vector<std::string> obtenerVecinos(const std::string &id) const;

    // En grafos no direccionados cada arista se devuelve una sola vez.
    std::vector<Arista> obtenerAristas() const;

    std::vector<Nodo> obtenerNodos() const;

    // Cambia el peso de una arista existente. Devuelve false si no existe.
    bool fijarPeso(const std::string &idOrigen, const std::string &idDestino, double peso);

    bool esDireccionado() const;
    bool esPonderado() const;

    std::size_t cantidadNodos() const;
    std::size_t cantidadAristas() const;

    void limpiar();
    void imprimir() const;

private:
    std::unordered_map<std::string, Nodo> nodos;
    std::unordered_map<std::string, std::vector<Arista>> aristas;
    bool direccionado;
    bool ponderado;
};

#endif // GRAFO_H
