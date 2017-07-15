#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QQueue>
#include"fortune.h"
#include<QGraphicsScene>
#include<QGraphicsView>


class MainWindow : public QMainWindow
{
    Q_OBJECT
    QGraphicsScene* scene;
    QGraphicsView* view;

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();
};

#endif // MAINWINDOW_H
