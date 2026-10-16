// Herramienta de entrenamiento del modelo de sentimiento fastText.
//
// Genera un modelo supervisado de fastText (Meta) a partir de las obras
// XML-TEI con etiquetado debil del lexico de polaridad. La logica vive en
// EntrenadorFastText, compartida con el dialogo de reentrenamiento de la
// aplicacion.
//
// Uso:
//   entrenarmodelo <carpeta_obras> <lexico.txt> <salida> [epochs]
//
// El modelo se guarda en <salida>.ftz y el archivo de entrenamiento en
// <salida>.train.txt. epochs es opcional (por defecto 5).

#include "entrenadorfasttext.h"

#include <QByteArray>
#include <QCoreApplication>
#include <QStringList>

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

int main(int argc, char *argv[])
{
    QCoreApplication aplicacion(argc, argv);

    const QStringList argumentos = QCoreApplication::arguments();

    if (argumentos.size() < 4)
    {
        std::cerr << "Uso: entrenarmodelo <carpeta_obras> <lexico.txt> <salida> [epochs]" << std::endl;
        std::cerr << "El modelo se guarda en <salida>.ftz y el archivo de entrenamiento en <salida>.train.txt." << std::endl;
        return 1;
    }

    const QString carpetaObras = argumentos.at(1);
    const QString rutaLexico = argumentos.at(2);
    std::string rutaSalida = argumentos.at(3).toStdString();
    int cantidadEpochs = 5;

    if (rutaSalida.size() >= 4 && rutaSalida.substr(rutaSalida.size() - 4) == ".bin")
    {
        rutaSalida.erase(rutaSalida.size() - 4);
    }

    if (argumentos.size() >= 5)
    {
        cantidadEpochs = argumentos.at(4).toInt();

        if (cantidadEpochs <= 0)
        {
            cantidadEpochs = 5;
        }
    }

    const QByteArray bytesLexico = rutaLexico.toLocal8Bit();
    std::ifstream archivoLexico(bytesLexico.constData());

    if (!archivoLexico.is_open())
    {
        std::cerr << "No se pudo abrir el lexico: " << rutaLexico.toStdString() << std::endl;
        return 1;
    }

    std::ostringstream contenido;
    contenido << archivoLexico.rdbuf();

    const EntrenadorFastText::Reportador reportar = [](const std::string &linea) {
        std::cout << linea << std::endl;
    };

    int ultimoDecimo = -1;

    const EntrenadorFastText::ReporteProgreso reportarProgreso = [&ultimoDecimo](int porcentaje) {
        const int decimo = porcentaje / 10;

        if (decimo != ultimoDecimo && decimo >= 0 && decimo <= 10)
        {
            ultimoDecimo = decimo;
            std::cout << "Progreso: " << (decimo * 10) << "%" << std::endl;
        }
    };

    EntrenadorFastText entrenador;

    const QByteArray bytesObras = carpetaObras.toLocal8Bit();

    const long muestras = entrenador.generarArchivoEntrenamiento(
        std::string(bytesObras.constData()),
        contenido.str(),
        rutaSalida + ".train.txt",
        reportar,
        reportarProgreso);

    if (muestras < 0)
    {
        return 1;
    }

    if (!entrenador.entrenar(rutaSalida + ".train.txt", rutaSalida, cantidadEpochs, 100, 2000000, reportar, reportarProgreso))
    {
        return 1;
    }

    return 0;
}
