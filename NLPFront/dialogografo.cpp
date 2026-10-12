#include "dialogografo.h"

#include <QBrush>
#include <QEvent>
#include <QFont>
#include <QGraphicsEllipseItem>
#include <QGraphicsLineItem>
#include <QGraphicsScene>
#include <QGraphicsSimpleTextItem>
#include <QGraphicsView>
#include <QHBoxLayout>
#include <QPainter>
#include <QPen>
#include <QPushButton>
#include <QShowEvent>
#include <QSize>
#include <QVBoxLayout>
#include <QWheelEvent>

#include <cmath>
#include <string>
#include <unordered_map>
#include <vector>

DialogoGrafo::DialogoGrafo(QWidget *parent)
    : QDialog(parent)
    , vista(new QGraphicsView(this))
    , escena(new QGraphicsScene(this))
{
    setWindowTitle(tr("Grafo de personajes"));
    resize(900, 650);

    vista->setScene(escena);
    vista->setRenderHint(QPainter::Antialiasing);
    vista->setDragMode(QGraphicsView::ScrollHandDrag);
    vista->setMinimumSize(200, 150);
    vista->viewport()->installEventFilter(this);

    QPushButton *botonAcercar = new QPushButton(tr("Acercar"), this);
    QPushButton *botonAlejar = new QPushButton(tr("Alejar"), this);
    QPushButton *botonAjustar = new QPushButton(tr("Ajustar"), this);

    connect(botonAcercar, &QPushButton::clicked, this, &DialogoGrafo::acercar);
    connect(botonAlejar, &QPushButton::clicked, this, &DialogoGrafo::alejar);
    connect(botonAjustar, &QPushButton::clicked, this, &DialogoGrafo::ajustarVista);

    QHBoxLayout *filaBotones = new QHBoxLayout();
    filaBotones->addWidget(botonAcercar);
    filaBotones->addWidget(botonAlejar);
    filaBotones->addWidget(botonAjustar);
    filaBotones->addStretch();

    QVBoxLayout *disposicion = new QVBoxLayout(this);
    disposicion->addLayout(filaBotones);
    disposicion->addWidget(vista);
}

void DialogoGrafo::mostrarGrafo(const Grafo &grafo)
{
    escena->clear();

    const std::vector<Nodo> nodos = grafo.obtenerNodos();

    if (nodos.empty())
    {
        escena->addSimpleText(tr("No hay personajes cargados."));
        escena->setSceneRect(-400.0, -300.0, 800.0, 600.0);
        vista->centerOn(0.0, 0.0);
        return;
    }

    const int total = static_cast<int>(nodos.size());
    const double radio = qMax(170.0, 26.0 * total);
    const double pi = 3.14159265358979323846;
    std::unordered_map<std::string, QPointF> posiciones;

    for (int i = 0; i < total; ++i)
    {
        const double angulo = (2.0 * pi * i) / total - pi / 2.0;
        posiciones[nodos[i].obtenerId()] = QPointF(radio * std::cos(angulo), radio * std::sin(angulo));
    }

    // Aristas: mas gruesas y mas oscuras cuanto mayor sea el peso.
    const std::vector<Arista> aristas = grafo.obtenerAristas();
    double pesoMaximo = 0.0;

    for (const Arista &arista : aristas)
    {
        if (arista.obtenerPeso() > pesoMaximo)
        {
            pesoMaximo = arista.obtenerPeso();
        }
    }

    if (pesoMaximo <= 0.0)
    {
        pesoMaximo = 1.0;
    }

    for (const Arista &arista : aristas)
    {
        const auto itOrigen = posiciones.find(arista.obtenerIdOrigen());
        const auto itDestino = posiciones.find(arista.obtenerIdDestino());

        if (itOrigen == posiciones.end() || itDestino == posiciones.end())
        {
            continue;
        }

        const double proporcion = arista.obtenerPeso() / pesoMaximo;
        const int tono = 215 - static_cast<int>(175.0 * proporcion);
        QPen lapiz(QColor(tono, tono, tono));
        lapiz.setWidthF(1.0 + 6.0 * proporcion);

        escena->addLine(QLineF(itOrigen->second, itDestino->second), lapiz);
    }

    // Nodos: elipse con el nombre del personaje.
    QFont fuente = font();
    fuente.setPointSize(9);

    for (const Nodo &nodo : nodos)
    {
        const QPointF centro = posiciones.at(nodo.obtenerId());
        QString nombre = QString::fromStdString(nodo.obtenerDato().obtenerValor());

        if (nombre.isEmpty())
        {
            nombre = QString::fromStdString(nodo.obtenerId());
        }

        QGraphicsSimpleTextItem *texto = escena->addSimpleText(nombre, fuente);
        const QRectF rectTexto = texto->boundingRect();
        const double rx = qMax(30.0, rectTexto.width() / 2.0 + 12.0);
        const double ry = qMax(24.0, rectTexto.height() / 2.0 + 10.0);

        QGraphicsEllipseItem *elipse = escena->addEllipse(
            centro.x() - rx, centro.y() - ry, 2.0 * rx, 2.0 * ry,
            QPen(QColor(50, 50, 50)), QBrush(QColor(214, 228, 240)));

        texto->setPos(centro.x() - rectTexto.width() / 2.0, centro.y() - rectTexto.height() / 2.0);
        texto->setBrush(QColor(20, 20, 20));
        elipse->setZValue(0.0);
        texto->setZValue(1.0);
    }

    escena->setSceneRect(escena->itemsBoundingRect().adjusted(-40.0, -40.0, 40.0, 40.0));
    ajustarVista();
}

void DialogoGrafo::ajustarVista()
{
    if (escena->sceneRect().isEmpty())
    {
        return;
    }

    const QSize tamano = vista->viewport()->size();

    if (tamano.width() <= 0 || tamano.height() <= 0)
    {
        return;
    }

    vista->fitInView(escena->sceneRect(), Qt::KeepAspectRatio);
}

void DialogoGrafo::acercar()
{
    vista->scale(1.25, 1.25);
}

void DialogoGrafo::alejar()
{
    vista->scale(0.8, 0.8);
}

bool DialogoGrafo::eventFilter(QObject *objeto, QEvent *evento)
{
    if (objeto == vista->viewport() && evento->type() == QEvent::Wheel)
    {
        const QWheelEvent *rueda = static_cast<const QWheelEvent *>(evento);
        const double factor = rueda->angleDelta().y() > 0 ? 1.15 : 1.0 / 1.15;

        vista->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
        vista->scale(factor, factor);
        vista->setTransformationAnchor(QGraphicsView::AnchorViewCenter);
        return true;
    }

    return QDialog::eventFilter(objeto, evento);
}

void DialogoGrafo::showEvent(QShowEvent *evento)
{
    QDialog::showEvent(evento);
    ajustarVista();
}
