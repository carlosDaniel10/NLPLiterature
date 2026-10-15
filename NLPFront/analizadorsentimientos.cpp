#include "analizadorsentimientos.h"

#include <cctype>
#include <fstream>
#include <sstream>
#include <vector>

namespace {

std::string recortar(const std::string &texto)
{
    const std::size_t primero = texto.find_first_not_of(" \t\r\n");

    if (primero == std::string::npos)
    {
        return std::string();
    }

    const std::size_t ultimo = texto.find_last_not_of(" \t\r\n");

    return texto.substr(primero, ultimo - primero + 1);
}

} // namespace

AnalizadorSentimientos::AnalizadorSentimientos()
    : lexico()
{
}

bool AnalizadorSentimientos::cargarLexico(const std::string &ruta)
{
    std::ifstream archivo(ruta);

    if (!archivo.is_open())
    {
        return false;
    }

    std::ostringstream contenido;
    contenido << archivo.rdbuf();
    cargarLexicoDesdeTexto(contenido.str());
    return true;
}

void AnalizadorSentimientos::cargarLexicoDesdeTexto(const std::string &contenido)
{
    std::istringstream flujo(contenido);
    std::string linea;

    while (std::getline(flujo, linea))
    {
        const std::string recortada = recortar(linea);

        if (recortada.empty() || recortada[0] == '#')
        {
            continue;
        }

        std::istringstream partes(recortada);
        std::string palabra;
        double puntaje = 0.0;

        if (partes >> palabra >> puntaje)
        {
            lexico[palabra] = puntaje;
        }
    }
}

double AnalizadorSentimientos::analizar(const std::string &texto) const
{
    double suma = 0.0;
    int totalPalabras = 0;
    int ventanaNegacion = 0;
    std::string palabra;

    for (std::size_t i = 0; i < texto.size();)
    {
        const unsigned char c = static_cast<unsigned char>(texto[i]);

        if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9'))
        {
            palabra += static_cast<char>(std::tolower(c));
            ++i;
        }
        else if (c == 0xC3 || c == 0xC4 || c == 0xC5)
        {
            // Letra con tilde en UTF-8: se conserva completa tal cual.
            if (i + 1 < texto.size())
            {
                palabra.append(texto, i, 2);
                i += 2;
            }
            else
            {
                ++i;
            }
        }
        else
        {
            if (!palabra.empty())
            {
                procesarPalabra(palabra, ventanaNegacion, suma, totalPalabras);
                palabra.clear();
            }

            ++i;
        }
    }

    if (!palabra.empty())
    {
        procesarPalabra(palabra, ventanaNegacion, suma, totalPalabras);
    }

    if (totalPalabras == 0)
    {
        return 0.0;
    }

    double puntaje = suma / totalPalabras;

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

std::size_t AnalizadorSentimientos::cantidadPalabras() const
{
    return lexico.size();
}

void AnalizadorSentimientos::procesarPalabra(const std::string &palabra, int &ventanaNegacion, double &suma, int &totalPalabras) const
{
    const auto it = lexico.find(palabra);

    if (it != lexico.end())
    {
        double valor = it->second;

        if (ventanaNegacion > 0)
        {
            valor = -valor;
        }

        suma += valor;
        ++totalPalabras;
    }

    if (esNegacion(palabra))
    {
        ventanaNegacion = 3;
    }
    else if (ventanaNegacion > 0)
    {
        --ventanaNegacion;
    }
}

bool AnalizadorSentimientos::esNegacion(const std::string &palabra) const
{
    static const std::vector<std::string> palabras =
        {"no", "nunca", "sin", "ni", "tampoco", "jamas", "nada", "nadie", "ningun", "ninguna", "ninguno"};

    for (const std::string &candidata : palabras)
    {
        if (palabra == candidata)
        {
            return true;
        }
    }

    return false;
}
