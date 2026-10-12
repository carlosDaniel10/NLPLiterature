#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include "dialogografo.h"

#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QStringList>
#include <QXmlStreamReader>

#include <algorithm>
#include <cstddef>
#include <string>
#include <unordered_map>
#include <vector>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , grafo(false, true)
    , dialogoGrafo(nullptr)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_botonLeerXmlTei_clicked()
{
    const QString ruta = QFileDialog::getOpenFileName(
        this,
        tr("Abrir archivo XML-TEI"),
        QString(),
        tr("Archivos XML-TEI (*.xml *.tei);;Todos los archivos (*)"));

    if (ruta.isEmpty())
    {
        return;
    }

    QFile archivo(ruta);

    if (!archivo.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QMessageBox::warning(this, tr("Error"), tr("No se pudo abrir el archivo."));
        return;
    }

    QXmlStreamReader lector(&archivo);
    QString texto;
    QString error;

    if (!extraerTextoTei(lector, texto, error))
    {
        QMessageBox::warning(this, tr("Error"), tr("No se pudo leer el XML: %1").arg(error));
        return;
    }

    ui->visorTexto->setPlainText(texto);

    grafo.limpiar();
    cargarPersonajes(ruta);
    cargarRelaciones(ruta);
    mostrarPersonajes();

    statusBar()->showMessage(tr("Archivo cargado: %1 (personajes: %2, relaciones: %3)")
                                 .arg(QFileInfo(ruta).fileName())
                                 .arg(grafo.cantidadNodos())
                                 .arg(grafo.cantidadAristas()));
}

void MainWindow::on_botonVerGrafo_clicked()
{
    if (grafo.cantidadNodos() == 0)
    {
        QMessageBox::information(this, tr("Grafo"), tr("No hay personajes cargados."));
        return;
    }

    if (dialogoGrafo == nullptr)
    {
        dialogoGrafo = new DialogoGrafo(this);
    }

    dialogoGrafo->mostrarGrafo(grafo);
    dialogoGrafo->show();
    dialogoGrafo->raise();
    dialogoGrafo->activateWindow();
}

bool MainWindow::extraerTextoTei(QXmlStreamReader &lector, QString &texto, QString &error)
{
    // Se omite el contenido de teiHeader para quedarse solo con el texto de la obra.
    int nivelEncabezado = 0;

    while (!lector.atEnd())
    {
        lector.readNext();

        if (lector.isStartElement())
        {
            if (lector.name() == QLatin1String("teiHeader"))
            {
                nivelEncabezado = 1;
            }
            else if (nivelEncabezado > 0)
            {
                ++nivelEncabezado;
            }
        }
        else if (lector.isEndElement())
        {
            if (nivelEncabezado > 0)
            {
                --nivelEncabezado;
            }
        }
        else if (nivelEncabezado == 0 && lector.isCharacters() && !lector.isWhitespace())
        {
            texto.append(lector.text().toString());
            texto.append(QLatin1Char(' '));
        }
    }

    if (lector.hasError())
    {
        error = lector.errorString();
        return false;
    }

    texto = texto.trimmed();
    return true;
}

void MainWindow::cargarPersonajes(const QString &ruta)
{
    QFile archivo(ruta);

    if (!archivo.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        return;
    }

    QXmlStreamReader lector(&archivo);
    bool dentroPersona = false;
    QString idPersona;
    QString nombrePersona;

    while (!lector.atEnd())
    {
        lector.readNext();

        if (lector.isStartElement())
        {
            const QString nombreElemento = lector.name().toString();

            if (nombreElemento == QLatin1String("person"))
            {
                dentroPersona = true;
                idPersona = lector.attributes().value(QLatin1String("xml:id")).toString();

                if (idPersona.isEmpty())
                {
                    idPersona = lector.attributes().value(QLatin1String("id")).toString();
                }

                nombrePersona.clear();
            }
            else if (nombreElemento == QLatin1String("persName") && nombrePersona.isEmpty())
            {
                if (dentroPersona)
                {
                    nombrePersona = lector.readElementText(QXmlStreamReader::IncludeChildElements).trimmed();
                }
                else
                {
                    const QString referencia = lector.attributes().value(QLatin1String("ref")).toString();

                    if (!referencia.isEmpty())
                    {
                        const QString textoPersona = lector.readElementText(QXmlStreamReader::IncludeChildElements).trimmed();
                        QString idReferencia = referencia;

                        if (idReferencia.startsWith(QLatin1Char('#')))
                        {
                            idReferencia.remove(0, 1);
                        }

                        if (!idReferencia.isEmpty() && !grafo.existeNodo(idReferencia.toStdString()))
                        {
                            const QString valor = textoPersona.isEmpty() ? idReferencia : textoPersona;
                            grafo.agregarNodo(Nodo(idReferencia.toStdString(), Dato(valor.toStdString())));
                        }
                    }
                }
            }
        }
        else if (lector.isEndElement() && dentroPersona && lector.name() == QLatin1String("person"))
        {
            QString idNodo = idPersona;

            if (idNodo.isEmpty())
            {
                idNodo = nombrePersona;
            }

            if (!idNodo.isEmpty() && !grafo.existeNodo(idNodo.toStdString()))
            {
                const QString valor = nombrePersona.isEmpty() ? idNodo : nombrePersona;
                grafo.agregarNodo(Nodo(idNodo.toStdString(), Dato(valor.toStdString())));
            }

            dentroPersona = false;
            idPersona.clear();
            nombrePersona.clear();
        }
    }
}

void MainWindow::cargarRelaciones(const QString &ruta)
{
    QFile archivo(ruta);

    if (!archivo.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        return;
    }

    std::unordered_map<std::string, std::string> idsPorNombre;

    for (const Nodo &nodo : grafo.obtenerNodos())
    {
        idsPorNombre[nodo.obtenerDato().obtenerValor()] = nodo.obtenerId();
    }

    QXmlStreamReader lector(&archivo);

    // Pila de unidades abiertas: cada parrafo/verso (narrativa) o escena
    // (drama) acumula los personajes que aparecen en el. escena[i] indica si
    // la unidad i es una escena de teatro.
    std::vector<std::vector<std::string>> unidades;
    std::vector<bool> escena;

    while (!lector.atEnd())
    {
        lector.readNext();

        const QString nombreElemento = lector.name().toString();

        if (lector.isStartElement())
        {
            if (nombreElemento == QLatin1String("p") || nombreElemento == QLatin1String("l"))
            {
                unidades.push_back(std::vector<std::string>());
                escena.push_back(false);
            }
            else if (nombreElemento == QLatin1String("div"))
            {
                const QString tipo = lector.attributes().value(QLatin1String("type")).toString();

                if (tipo == QLatin1String("scene"))
                {
                    unidades.push_back(std::vector<std::string>());
                    escena.push_back(true);
                }
            }
            else if (nombreElemento == QLatin1String("sp") && !unidades.empty())
            {
                // En drama, los personajes que hablan en la escena (who).
                const QString quien = lector.attributes().value(QLatin1String("who")).toString();
                const QStringList ids = quien.split(QLatin1Char(' '), Qt::SkipEmptyParts);

                for (const QString &token : ids)
                {
                    QString id = token;

                    if (id.startsWith(QLatin1Char('#')))
                    {
                        id.remove(0, 1);
                    }

                    const std::string idStd = id.toStdString();

                    if (!idStd.empty() && grafo.existeNodo(idStd)
                        && std::find(unidades.back().begin(), unidades.back().end(), idStd) == unidades.back().end())
                    {
                        unidades.back().push_back(idStd);
                    }
                }
            }
            else if (nombreElemento == QLatin1String("persName") && !unidades.empty())
            {
                const QString referencia = lector.attributes().value(QLatin1String("ref")).toString();
                const QString textoPersona = lector.readElementText(QXmlStreamReader::IncludeChildElements).trimmed();
                std::string idMencion;

                if (!referencia.isEmpty())
                {
                    QString idReferencia = referencia;

                    if (idReferencia.startsWith(QLatin1Char('#')))
                    {
                        idReferencia.remove(0, 1);
                    }

                    idMencion = idReferencia.toStdString();
                }
                else
                {
                    const auto it = idsPorNombre.find(textoPersona.toStdString());

                    if (it != idsPorNombre.end())
                    {
                        idMencion = it->second;
                    }
                }

                if (!idMencion.empty() && grafo.existeNodo(idMencion)
                    && std::find(unidades.back().begin(), unidades.back().end(), idMencion) == unidades.back().end())
                {
                    unidades.back().push_back(idMencion);
                }
            }
        }
        else if (lector.isEndElement())
        {
            const QString nombreElemento = lector.name().toString();
            bool cerrar = false;

            if (nombreElemento == QLatin1String("p") || nombreElemento == QLatin1String("l"))
            {
                cerrar = !unidades.empty();
            }
            else if (nombreElemento == QLatin1String("div") && !escena.empty() && escena.back())
            {
                cerrar = true;
            }

            if (cerrar)
            {
                registrarInteracciones(unidades.back());
                unidades.pop_back();
                escena.pop_back();
            }
        }
    }
}

void MainWindow::registrarInteracciones(const std::vector<std::string> &menciones)
{
    // Cada unidad donde coinciden dos personajes suma 1.0 al peso de su relacion.
    for (std::size_t i = 0; i < menciones.size(); ++i)
    {
        for (std::size_t j = i + 1; j < menciones.size(); ++j)
        {
            grafo.aumentarPeso(menciones[i], menciones[j]);
        }
    }
}

void MainWindow::mostrarPersonajes()
{
    ui->listaPersonajes->clear();

    QStringList personajes;

    for (const Nodo &nodo : grafo.obtenerNodos())
    {
        const QString id = QString::fromStdString(nodo.obtenerId());
        const QString nombre = QString::fromStdString(nodo.obtenerDato().obtenerValor());
        QString texto = nombre;

        if (nombre != id)
        {
            texto = QString("%1 (%2)").arg(nombre, id);
        }

        personajes.append(texto);
    }

    personajes.sort(Qt::CaseInsensitive);

    for (const QString &personaje : personajes)
    {
        ui->listaPersonajes->addItem(personaje);
    }
}
