#include "dialogoentrenar.h"
#include "./ui_dialogoentrenar.h"

#include "entrenadorfasttext.h"

#include <QCloseEvent>
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QThread>

#include <string>

// Trabaja en un hilo aparte: ejecuta las fases de entrenamiento y emite
// progreso, log y resultado hacia el dialogo.
class TrabajadorEntrenamiento : public QObject
{
    Q_OBJECT

public:
    TrabajadorEntrenamiento(bool modoCompleto,
                            const QString &carpetaObras,
                            const QString &rutaTrain,
                            const QString &salida,
                            int epochs,
                            int dim,
                            int bucket,
                            QObject *parent = nullptr)
        : QObject(parent)
        , modoCompleto(modoCompleto)
        , carpetaObras(carpetaObras)
        , rutaTrain(rutaTrain)
        , salida(salida)
        , epochs(epochs)
        , dim(dim)
        , bucket(bucket)
    {
    }

public slots:
    void ejecutar();

signals:
    void progreso(int porcentaje);
    void log(const QString &linea);
    void terminado(bool exito, const QString &mensaje, const QString &rutaModelo);

private:
    bool modoCompleto;
    QString carpetaObras;
    QString rutaTrain;
    QString salida;
    int epochs;
    int dim;
    int bucket;
};

void TrabajadorEntrenamiento::ejecutar()
{
    const QByteArray bytesSalida = salida.toLocal8Bit();
    const std::string salidaStd(bytesSalida.constData());
    std::string rutaTrainStd;

    EntrenadorFastText entrenador;

    const EntrenadorFastText::Reportador reportar = [this](const std::string &linea) {
        emit log(QString::fromStdString(linea));
    };

    const EntrenadorFastText::ReporteProgreso reportarProgreso = [this](int porcentaje) {
        emit progreso(porcentaje);
    };

    if (modoCompleto)
    {
        // Lexico desde el recurso integrado de la aplicacion.
        QFile archivoLexico(QStringLiteral(":/sentimientos_es.txt"));

        if (!archivoLexico.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            emit terminado(false, tr("No se pudo leer el lexico del recurso integrado."), QString());
            return;
        }

        const std::string contenidoLexico = QString::fromUtf8(archivoLexico.readAll()).toStdString();
        rutaTrainStd = salidaStd + ".train.txt";

        const QByteArray bytesObras = carpetaObras.toLocal8Bit();
        const long muestras = entrenador.generarArchivoEntrenamiento(
            std::string(bytesObras.constData()), contenidoLexico, rutaTrainStd, reportar, reportarProgreso);

        if (muestras < 0)
        {
            emit terminado(false, tr("No se pudo generar el archivo de entrenamiento."), QString());
            return;
        }
    }
    else
    {
        const QByteArray bytesTrain = rutaTrain.toLocal8Bit();
        rutaTrainStd.assign(bytesTrain.constData());

        // Validacion rapida: el archivo debe existir y tener etiquetas.
        QFile archivo(rutaTrain);

        if (!archivo.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            emit terminado(false, tr("No se pudo abrir el archivo train.txt."), QString());
            return;
        }

        long positivas = 0;
        long negativas = 0;
        long neutrales = 0;
        long total = 0;

        while (!archivo.atEnd())
        {
            const QByteArray linea = archivo.readLine().trimmed();

            if (linea.startsWith("__label__positivo"))
            {
                ++positivas;
                ++total;
            }
            else if (linea.startsWith("__label__negativo"))
            {
                ++negativas;
                ++total;
            }
            else if (linea.startsWith("__label__neutral"))
            {
                ++neutrales;
                ++total;
            }
        }

        if (total == 0)
        {
            emit terminado(false, tr("El archivo no tiene lineas con etiqueta __label__."), QString());
            return;
        }

        emit log(tr("Clases encontradas: %1 positivas, %2 negativas, %3 neutrales (%4 lineas).")
                     .arg(positivas)
                     .arg(negativas)
                     .arg(neutrales)
                     .arg(total));
    }

    if (!entrenador.entrenar(rutaTrainStd, salidaStd, epochs, dim, bucket, reportar, reportarProgreso))
    {
        emit terminado(false, tr("El entrenamiento fallo. Revise el registro."), QString());
        return;
    }

    emit terminado(true, tr("Modelo reentrenado correctamente."), QString::fromStdString(salidaStd + ".ftz"));
}

DialogoEntrenar::DialogoEntrenar(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::DialogoEntrenar)
    , hilo(nullptr)
    , corriendo(false)
{
    ui->setupUi(this);

    const QString carpetaAplicacion = QCoreApplication::applicationDirPath();

    const QString candidataObras = carpetaAplicacion + QStringLiteral("/../../../Obras");

    if (QFileInfo(candidataObras).isDir())
    {
        ui->editarCarpetaObras->setText(QDir::cleanPath(candidataObras));
    }

    const QString carpetaFuente = carpetaAplicacion + QStringLiteral("/../..");

    if (QFileInfo(carpetaFuente).isDir())
    {
        ui->editarSalida->setText(QDir::cleanPath(carpetaFuente + QStringLiteral("/sentimientos_modelo")));
    }
    else
    {
        ui->editarSalida->setText(carpetaAplicacion + QStringLiteral("/sentimientos_modelo"));
    }
}

DialogoEntrenar::~DialogoEntrenar()
{
    if (hilo != nullptr)
    {
        hilo->wait();
    }

    delete ui;
}

void DialogoEntrenar::on_radioCompleto_toggled(bool activo)
{
    ui->pilaPaginas->setCurrentIndex(activo ? 0 : 1);
}

void DialogoEntrenar::on_botonExaminarObras_clicked()
{
    const QString carpeta = QFileDialog::getExistingDirectory(
        this, tr("Carpeta de obras"), ui->editarCarpetaObras->text());

    if (!carpeta.isEmpty())
    {
        ui->editarCarpetaObras->setText(carpeta);
    }
}

void DialogoEntrenar::on_botonExaminarTrain_clicked()
{
    const QString ruta = QFileDialog::getOpenFileName(
        this,
        tr("Archivo de entrenamiento"),
        ui->editarArchivoTrain->text(),
        tr("Archivos de entrenamiento (*.txt);;Todos los archivos (*)"));

    if (!ruta.isEmpty())
    {
        ui->editarArchivoTrain->setText(ruta);
    }
}

void DialogoEntrenar::on_botonExaminarSalida_clicked()
{
    const QString ruta = QFileDialog::getSaveFileName(
        this,
        tr("Modelo de salida"),
        ui->editarSalida->text(),
        tr("Modelo fastText (*.ftz)"));

    if (!ruta.isEmpty())
    {
        ui->editarSalida->setText(ruta);
    }
}

void DialogoEntrenar::on_botonIniciar_clicked()
{
    if (corriendo)
    {
        return;
    }

    QString carpetaObras;
    QString rutaTrain;
    QString salida = ui->editarSalida->text().trimmed();

    if (salida.endsWith(QStringLiteral(".ftz")))
    {
        salida.chop(4);
    }
    else if (salida.endsWith(QStringLiteral(".bin")))
    {
        salida.chop(4);
    }

    if (salida.isEmpty())
    {
        QMessageBox::warning(this, tr("Reentrenar modelo"), tr("Indique la ruta de salida del modelo."));
        return;
    }

    if (ui->radioCompleto->isChecked())
    {
        carpetaObras = ui->editarCarpetaObras->text().trimmed();

        if (carpetaObras.isEmpty() || !QFileInfo(carpetaObras).isDir())
        {
            QMessageBox::warning(this, tr("Reentrenar modelo"), tr("Seleccione una carpeta de obras valida."));
            return;
        }
    }
    else
    {
        rutaTrain = ui->editarArchivoTrain->text().trimmed();

        if (rutaTrain.isEmpty() || !QFileInfo::exists(rutaTrain))
        {
            QMessageBox::warning(this, tr("Reentrenar modelo"), tr("Seleccione un archivo train.txt valido."));
            return;
        }
    }

    corriendo = true;
    ui->botonIniciar->setEnabled(false);
    ui->radioCompleto->setEnabled(false);
    ui->radioTrain->setEnabled(false);
    ui->botonExaminarObras->setEnabled(false);
    ui->botonExaminarTrain->setEnabled(false);
    ui->botonExaminarSalida->setEnabled(false);
    ui->editarCarpetaObras->setEnabled(false);
    ui->editarArchivoTrain->setEnabled(false);
    ui->editarSalida->setEnabled(false);
    ui->giroEpochs->setEnabled(false);
    ui->giroDim->setEnabled(false);
    ui->giroBucket->setEnabled(false);

    ui->registro->clear();
    ui->barraProgreso->setRange(0, 100);
    ui->barraProgreso->setValue(0);

    TrabajadorEntrenamiento *trabajador = new TrabajadorEntrenamiento(
        ui->radioCompleto->isChecked(),
        carpetaObras,
        rutaTrain,
        salida,
        ui->giroEpochs->value(),
        ui->giroDim->value(),
        ui->giroBucket->value());

    hilo = new QThread(this);
    trabajador->moveToThread(hilo);

    connect(hilo, &QThread::started, trabajador, &TrabajadorEntrenamiento::ejecutar);
    connect(trabajador, &TrabajadorEntrenamiento::progreso, this, &DialogoEntrenar::alProgreso);
    connect(trabajador, &TrabajadorEntrenamiento::log, this, &DialogoEntrenar::alLog);
    connect(trabajador, &TrabajadorEntrenamiento::terminado, this, &DialogoEntrenar::alTerminado);
    // Conexion directa: quit() debe ejecutarse en el hilo trabajador antes de
    // que el dialogo espere con wait(); una conexion encolada se procesaria
    // tarde y produciria un bloqueo.
    connect(trabajador, &TrabajadorEntrenamiento::terminado, hilo, &QThread::quit, Qt::DirectConnection);
    connect(hilo, &QThread::finished, trabajador, &QObject::deleteLater);
    connect(hilo, &QThread::finished, hilo, &QObject::deleteLater);

    hilo->start();
}

void DialogoEntrenar::on_botonCerrar_clicked()
{
    reject();
}

void DialogoEntrenar::alProgreso(int porcentaje)
{
    ui->barraProgreso->setValue(porcentaje);
}

void DialogoEntrenar::alLog(const QString &linea)
{
    ui->registro->appendPlainText(linea);
}

void DialogoEntrenar::alTerminado(bool exito, const QString &mensaje, const QString &rutaModelo)
{
    corriendo = false;

    // Espera a que el hilo termine su bucle antes de liberar la referencia;
    // evita destruir un QThread en ejecucion si el usuario cierra el dialogo
    // justo al terminar.
    if (hilo != nullptr)
    {
        hilo->wait();
        hilo = nullptr;
    }

    ui->botonIniciar->setEnabled(true);
    ui->radioCompleto->setEnabled(true);
    ui->radioTrain->setEnabled(true);
    ui->botonExaminarObras->setEnabled(true);
    ui->botonExaminarTrain->setEnabled(true);
    ui->botonExaminarSalida->setEnabled(true);
    ui->editarCarpetaObras->setEnabled(true);
    ui->editarArchivoTrain->setEnabled(true);
    ui->editarSalida->setEnabled(true);
    ui->giroEpochs->setEnabled(true);
    ui->giroDim->setEnabled(true);
    ui->giroBucket->setEnabled(true);

    ui->registro->appendPlainText(mensaje);

    if (!exito)
    {
        return;
    }

    ui->barraProgreso->setValue(100);
    emit modeloEntrenado(rutaModelo);
}

void DialogoEntrenar::closeEvent(QCloseEvent *evento)
{
    if (corriendo)
    {
        QMessageBox::information(this, tr("Reentrenar modelo"),
                                 tr("El entrenamiento esta en curso. Espere a que termine."));
        evento->ignore();
        return;
    }

    QDialog::closeEvent(evento);
}

#include "dialogoentrenar.moc"
