#ifndef DIALOGOGRAFO_H
#define DIALOGOGRAFO_H

#include "grafo.h"

#include <QDialog>

QT_BEGIN_NAMESPACE
class QGraphicsScene;
class QGraphicsView;
class QResizeEvent;
class QShowEvent;
QT_END_NAMESPACE

// Ventana que dibuja el grafo: cada nodo es un personaje y las aristas se
// ven mas gruesas y oscuras cuanto mayor es su peso.
class DialogoGrafo : public QDialog
{
    Q_OBJECT

public:
    explicit DialogoGrafo(QWidget *parent = nullptr);

    void mostrarGrafo(const Grafo &grafo);

protected:
    void showEvent(QShowEvent *evento) override;
    void resizeEvent(QResizeEvent *evento) override;

private:
    void ajustarVista();

    QGraphicsView *vista;
    QGraphicsScene *escena;
};

#endif // DIALOGOGRAFO_H
