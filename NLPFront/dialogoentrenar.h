#ifndef DIALOGOENTRENAR_H
#define DIALOGOENTRENAR_H

#include <QDialog>
#include <QString>

QT_BEGIN_NAMESPACE
class QCloseEvent;
class QThread;
namespace Ui {
class DialogoEntrenar;
}
QT_END_NAMESPACE

// Dialogo de reentrenamiento del modelo fastText. Soporta dos modos:
// proceso completo desde las obras o solo desde un archivo train.txt.
// El entrenamiento corre en un hilo aparte para no congelar la interfaz.
class DialogoEntrenar : public QDialog
{
    Q_OBJECT

public:
    explicit DialogoEntrenar(QWidget *parent = nullptr);
    ~DialogoEntrenar();

signals:
    // Se emite cuando el entrenamiento termino con exito; rutaModelo es el
    // archivo .ftz generado.
    void modeloEntrenado(const QString &rutaModelo);

private slots:
    void on_radioCompleto_toggled(bool activo);
    void on_botonExaminarObras_clicked();
    void on_botonExaminarTrain_clicked();
    void on_botonExaminarSalida_clicked();
    void on_botonIniciar_clicked();
    void on_botonCerrar_clicked();
    void alProgreso(int porcentaje);
    void alLog(const QString &linea);
    void alTerminado(bool exito, const QString &mensaje, const QString &rutaModelo);

private:
    void iniciarEntrenamiento();
    void closeEvent(QCloseEvent *evento) override;

    Ui::DialogoEntrenar *ui;
    QThread *hilo;
    bool corriendo;
};

#endif // DIALOGOENTRENAR_H
