#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QXmlStreamReader>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
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
    statusBar()->showMessage(tr("Archivo cargado: %1").arg(QFileInfo(ruta).fileName()));
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
