#include "analizadorfasttext.h"

#include "fasttext.h"

#include <algorithm>
#include <exception>
#include <sstream>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace {

const char *etiquetaPositiva = "__label__positivo";
const char *etiquetaNegativa = "__label__negativo";

} // namespace

class AnalizadorFastText::Implementacion
{
public:
    Implementacion()
        : modelo()
        , disponible(false)
    {
    }

    fasttext::FastText &obtenerModelo()
    {
        return modelo;
    }

    bool estaDisponible() const
    {
        return disponible;
    }

    void fijarDisponible(bool valor)
    {
        disponible = valor;
    }

private:
    fasttext::FastText modelo;
    bool disponible;
};

AnalizadorFastText::AnalizadorFastText()
    : implementacion(std::make_unique<Implementacion>())
{
}

AnalizadorFastText::~AnalizadorFastText() = default;

bool AnalizadorFastText::cargarModelo(const std::string &ruta)
{
    try
    {
        implementacion->obtenerModelo().loadModel(ruta);
        implementacion->fijarDisponible(true);
    }
    catch (const std::exception &)
    {
        implementacion->fijarDisponible(false);
    }

    return implementacion->estaDisponible();
}

bool AnalizadorFastText::estaDisponible() const
{
    return implementacion->estaDisponible();
}

double AnalizadorFastText::analizar(const std::string &texto)
{
    if (!implementacion->estaDisponible() || texto.empty())
    {
        return 0.0;
    }

    try
    {
        // predictLine lee una linea del flujo y devuelve las k etiquetas mas
        // probables. El texto se aplanar para que no queden saltos de linea.
        std::string aplanado = texto;
        std::replace(aplanado.begin(), aplanado.end(), '\n', ' ');
        std::istringstream flujo(aplanado);
        std::vector<std::pair<fasttext::real, std::string>> resultados;

        if (!implementacion->obtenerModelo().predictLine(flujo, resultados, 3, 0.0f))
        {
            return 0.0;
        }

        std::unordered_map<std::string, double> probabilidades;

        for (const auto &par : resultados)
        {
            probabilidades[par.second] = static_cast<double>(par.first);
        }

        const auto itPositivo = probabilidades.find(etiquetaPositiva);
        const auto itNegativo = probabilidades.find(etiquetaNegativa);
        const double positivo = (itPositivo == probabilidades.end()) ? 0.0 : itPositivo->second;
        const double negativo = (itNegativo == probabilidades.end()) ? 0.0 : itNegativo->second;

        double puntaje = positivo - negativo;

        if (puntaje > 1.0)
        {
            puntaje = 1.0;
        }
        else if (puntaje < -1.0)
        {
            puntaje = -1.0;
        }

        return puntaje;
    }
    catch (const std::exception &)
    {
        return 0.0;
    }
}
