#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "grafo.h"

#include <string>
#include <vector>

#include <QMainWindow>
#include <QString>

QT_BEGIN_NAMESPACE
class QXmlStreamReader;
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class DialogoGrafo;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_botonLeerXmlTei_clicked();
    void on_botonVerGrafo_clicked();

private:
    bool extraerTextoTei(QXmlStreamReader &lector, QString &texto, QString &error);
    void cargarPersonajes(const QString &ruta);
    void cargarRelaciones(const QString &ruta);
    void registrarInteracciones(const std::vector<std::string> &menciones);
    void mostrarPersonajes();

    Ui::MainWindow *ui;
    Grafo grafo;
    DialogoGrafo *dialogoGrafo;
};
#endif // MAINWINDOW_H
