#include "entrenadorfasttext.h"

#include "analizadorlexico.h"

#include "fasttext.h"

#include <QDir>
#include <QFile>
#include <QString>
#include <QStringList>
#include <QXmlStreamReader>

#include <cstddef>
#include <cstdio>
#include <exception>
#include <fstream>
#include <string>
#include <vector>

namespace {

const double umbralPositivo = 0.25;
const double umbralNegativo = -0.25;
const int minimoPalabras = 4;
// Limite de muestras por obra para mantener el corpus acotado.
const std::size_t limiteMuestrasPorObra = 250;

std::string aplanar(const QString &texto)
{
    return texto.simplified().toStdString();
}

int contarPalabras(const std::string &texto)
{
    int total = 0;
    bool dentroPalabra = false;

    for (const char c : texto)
    {
        if (c == ' ' || c == '\t' || c == '\n' || c == '\r')
        {
            dentroPalabra = false;
        }
        else if (!dentroPalabra)
        {
            dentroPalabra = true;
            ++total;
        }
    }

    return total;
}

std::string etiquetaDesdePuntaje(double puntaje)
{
    if (puntaje >= umbralPositivo)
    {
        return "positivo";
    }

    if (puntaje <= umbralNegativo)
    {
        return "negativo";
    }

    return "neutral";
}

void agregarUnidad(const QString &texto,
                   AnalizadorLexico &lexico,
                   std::vector<std::string> &etiquetas,
                   std::vector<std::string> &textos)
{
    const std::string aplanado = aplanar(texto);

    if (contarPalabras(aplanado) < minimoPalabras)
    {
        return;
    }

    etiquetas.push_back(etiquetaDesdePuntaje(lexico.analizar(aplanado)));
    textos.push_back(aplanado);
}

// Recorre el XML-TEI y extrae las unidades de texto (parlamentos de drama,
// parrafos y versos de narrativa) como muestras de entrenamiento.
bool extraerUnidades(const QString &ruta,
                     AnalizadorLexico &lexico,
                     std::vector<std::string> &etiquetas,
                     std::vector<std::string> &textos,
                     const EntrenadorFastText::Reportador &reportar)
{
    QFile archivo(ruta);

    if (!archivo.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        reportar("No se pudo abrir " + ruta.toStdString());
        return false;
    }

    QXmlStreamReader lector(&archivo);
    int nivelEscena = 0;
    int nivelEncabezado = 0;
    bool dentroUnidad = false;
    QString textoUnidad;
    const std::size_t muestrasIniciales = etiquetas.size();

    while (!lector.atEnd())
    {
        lector.readNext();

        if (lector.isStartElement())
        {
            const QString nombreElemento = lector.name().toString();

            if (nombreElemento == QLatin1String("teiHeader"))
            {
                nivelEncabezado = 1;
                continue;
            }

            if (nivelEncabezado > 0)
            {
                ++nivelEncabezado;
                continue;
            }

            if (nombreElemento == QLatin1String("div"))
            {
                const QString tipo = lector.attributes().value(QLatin1String("type")).toString();

                if (tipo == QLatin1String("scene"))
                {
                    ++nivelEscena;
                }
            }
            else if (nombreElemento == QLatin1String("sp") && nivelEscena > 0)
            {
                // Parlamento de drama: muestra propia.
                const QString textoParlamento = lector.readElementText(QXmlStreamReader::IncludeChildElements).trimmed();
                agregarUnidad(textoParlamento, lexico, etiquetas, textos);

                if (etiquetas.size() - muestrasIniciales >= limiteMuestrasPorObra)
                {
                    break;
                }
            }
            else if ((nombreElemento == QLatin1String("p") || nombreElemento == QLatin1String("l")) && nivelEscena == 0)
            {
                dentroUnidad = true;
                textoUnidad.clear();
            }
        }
        else if (lector.isEndElement())
        {
            const QString nombreElemento = lector.name().toString();

            if (nivelEncabezado > 0)
            {
                --nivelEncabezado;
                continue;
            }

            if (nombreElemento == QLatin1String("div") && nivelEscena > 0)
            {
                --nivelEscena;
            }
            else if ((nombreElemento == QLatin1String("p") || nombreElemento == QLatin1String("l")) && dentroUnidad)
            {
                agregarUnidad(textoUnidad, lexico, etiquetas, textos);
                dentroUnidad = false;
                textoUnidad.clear();

                if (etiquetas.size() - muestrasIniciales >= limiteMuestrasPorObra)
                {
                    break;
                }
            }
        }
        else if (lector.isCharacters() && dentroUnidad && !lector.isWhitespace())
        {
            textoUnidad.append(lector.text().toString());
            textoUnidad.append(QLatin1Char(' '));
        }
    }

    if (lector.hasError())
    {
        reportar("Error al leer " + ruta.toStdString() + ": " + lector.errorString().toStdString());
        return false;
    }

    return true;
}

// Escribe el archivo de entrenamiento limitando la cantidad de neutrales
// para que no dominen la muestra. Devuelve la cantidad de lineas escritas
// o -1 en caso de error.
long escribirMuestras(const std::string &ruta,
                      const std::vector<std::string> &etiquetas,
                      const std::vector<std::string> &textos,
                      const EntrenadorFastText::Reportador &reportar)
{
    std::size_t positivas = 0;
    std::size_t negativas = 0;
    std::size_t neutrales = 0;

    for (const std::string &etiqueta : etiquetas)
    {
        if (etiqueta == "positivo")
        {
            ++positivas;
        }
        else if (etiqueta == "negativo")
        {
            ++negativas;
        }
        else
        {
            ++neutrales;
        }
    }

    reportar("Muestras: " + std::to_string(positivas) + " positivas, "
             + std::to_string(negativas) + " negativas, "
             + std::to_string(neutrales) + " neutrales.");

    const std::size_t limiteNeutrales = 3 * (positivas + negativas) + 50;
    std::size_t paso = 1;

    if (neutrales > limiteNeutrales)
    {
        paso = neutrales / limiteNeutrales + 1;
    }

    std::ofstream archivo(ruta);

    if (!archivo.is_open())
    {
        reportar("No se pudo escribir " + ruta);
        return -1;
    }

    std::size_t neutralesRecorridas = 0;
    long escritas = 0;

    for (std::size_t i = 0; i < textos.size(); ++i)
    {
        if (etiquetas[i] == "neutral")
        {
            if (neutralesRecorridas % paso != 0)
            {
                ++neutralesRecorridas;
                continue;
            }

            ++neutralesRecorridas;
        }

        archivo << "__label__" << etiquetas[i] << " " << textos[i] << "\n";
        ++escritas;
    }

    reportar("Ejemplos escritos en " + ruta + ": " + std::to_string(escritas));
    return escritas;
}

} // namespace

EntrenadorFastText::EntrenadorFastText() = default;

long EntrenadorFastText::generarArchivoEntrenamiento(const std::string &carpetaObras,
                                                     const std::string &contenidoLexico,
                                                     const std::string &rutaTrain,
                                                     const Reportador &reportar,
                                                     const ReporteProgreso &reportarProgreso)
{
    AnalizadorLexico lexico;
    lexico.cargarLexicoDesdeTexto(contenidoLexico);
    reportar("Lexico cargado: " + std::to_string(lexico.cantidadPalabras()) + " palabras.");

    QDir carpeta(QString::fromStdString(carpetaObras));
    const QStringList filtros = QStringList() << QStringLiteral("*.xml") << QStringLiteral("*.tei");
    const QStringList archivos = carpeta.entryList(filtros, QDir::Files);

    if (archivos.isEmpty())
    {
        reportar("No hay archivos XML en " + carpetaObras);
        return -1;
    }

    std::vector<std::string> etiquetas;
    std::vector<std::string> textos;

    for (int i = 0; i < archivos.size(); ++i)
    {
        const QString rutaCompleta = carpeta.filePath(archivos.at(i));
        reportar("Procesando " + archivos.at(i).toStdString() + "...");

        if (!extraerUnidades(rutaCompleta, lexico, etiquetas, textos, reportar))
        {
            return -1;
        }

        reportarProgreso(static_cast<int>(30.0 * (i + 1) / archivos.size()));
    }

    if (textos.size() < 20)
    {
        reportar("Muy pocas muestras (" + std::to_string(textos.size()) + ").");
        return -1;
    }

    return escribirMuestras(rutaTrain, etiquetas, textos, reportar);
}

bool EntrenadorFastText::entrenar(const std::string &rutaTrain,
                                  const std::string &salida,
                                  int epochs,
                                  int dim,
                                  int bucket,
                                  const Reportador &reportar,
                                  const ReporteProgreso &reportarProgreso)
{
    fasttext::Args parametros;
    parametros.input = rutaTrain;
    parametros.output = salida;
    parametros.model = fasttext::model_name::sup;
    parametros.dim = dim;
    parametros.epoch = epochs;
    parametros.lr = 0.5;
    parametros.wordNgrams = 2;
    parametros.minCount = 2;
    parametros.bucket = bucket;
    parametros.loss = fasttext::loss_name::softmax;
    parametros.thread = 4;
    // verbose > 1 es necesario para que fastText actualice el loss que
    // recibe el callback de progreso; su salida va a stderr, que en la
    // aplicacion grafica no tiene consola.
    parametros.verbose = 2;

    reportar("Entrenando modelo fastText (" + std::to_string(epochs) + " epochs)...");

    fasttext::FastText modelo;
    int ultimoDecimo = -1;

    try
    {
        modelo.train(parametros, [&](float progreso, float perdida, int32_t, double, double) {
            const int decimo = static_cast<int>(progreso * 10.0f);

            if (decimo != ultimoDecimo)
            {
                ultimoDecimo = decimo;
                reportar("Entrenando... " + std::to_string(decimo * 10) + "% - perdida "
                         + std::to_string(perdida));
            }

            reportarProgreso(30 + static_cast<int>(55.0f * progreso));
        });
    }
    catch (const std::exception &error)
    {
        reportar("Error al entrenar: " + std::string(error.what()));
        return false;
    }

    reportar("Cuantizando modelo...");
    fasttext::Args cuantizar;
    cuantizar.input = rutaTrain;
    cuantizar.qout = false;
    cuantizar.dsub = 4;

    try
    {
        modelo.quantize(cuantizar, [&](float progreso, float, int32_t, double, double) {
            reportarProgreso(85 + static_cast<int>(15.0f * progreso));
        });
    }
    catch (const std::exception &error)
    {
        reportar("Error al cuantizar: " + std::string(error.what()));
        return false;
    }

    modelo.saveModel(salida + ".ftz");
    reportar("Modelo guardado en " + salida + ".ftz");

    // Restos de corridas anteriores sin cuantizar: se eliminan.
    std::remove((salida + ".bin").c_str());
    std::remove((salida + ".vec").c_str());

    reportarProgreso(100);
    return true;
}
