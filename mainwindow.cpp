#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{

    Fortune f;
    f.startAlgorithm();
    int a = 0;
}

MainWindow::~MainWindow()
{

}
