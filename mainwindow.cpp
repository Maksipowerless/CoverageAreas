#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{

    this->setGeometry(100,100,600,600);
    scene = new QGraphicsScene;
    view = new QGraphicsView(this);
    view->setGeometry(0,0,600,600);
    view->setScene(scene);
    view->setDragMode(QGraphicsView::ScrollHandDrag);

    Fortune f;
    f.loadFromFile(this);
    f.startAlgorithm(scene);
    f.printOutput(scene);

}

MainWindow::~MainWindow()
{

}
