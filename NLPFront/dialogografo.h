#ifndef DIALOGOGRAFO_H
#define DIALOGOGRAFO_H

#include "grafo.h"

#include <QDialog>

QT_BEGIN_NAMESPACE
class QEvent;
class QGraphicsScene;
class QGraphicsView;
class QObject;
class QShowEvent;
QT_END_NAMESPACE

// Ventana que dibuja el grafo: cada nodo es un personaje y las aristas se
// ven mas gruesas y oscuras cuanto mayor es su peso.
// Permite hacer zoom con los botones o con la rueda del raton.
class DialogoGrafo : public QDialog
{
    Q_OBJECT

public:
    explicit DialogoGrafo(QWidget *parent = nullptr);

    void mostrarGrafo(const Grafo &grafo);

protected:
    void showEvent(QShowEvent *evento) override;
    bool eventFilter(QObject *objeto, QEvent *evento) override;

private slots:
    void acercar();
    void alejar();

private:
    void ajustarVista();

    QGraphicsView *vista;
    QGraphicsScene *escena;
};

#endif // DIALOGOGRAFO_H
