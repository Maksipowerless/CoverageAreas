#ifndef FORTUNE_H
#define FORTUNE_H

#include"additinalstruct.h"
#include"tree.h"
#include"mypoint.h"
#include<QPointF>
#include<QGraphicsScene>
#include<QMap>
#include<cohensutherland.h>
#include<QMultiMap>
#include<QLinkedList>
#include<QFile>
#include<QTextStream>

//Cтруктура для хранения данных о вышке.
struct properties{
    int id;
    int tP;
    properties()
        :id(0), tP(0){}
    properties(int i, int p)
        : id(i),tP(p) {}
    properties operator=(const properties& p1){id = p1.id; tP = p1.tP; return *this;}
};

class Fortune{
    double X0 = 0, X1 = 0, Y0 = 0, Y1 = 0;//Координаты ограничивающей области.
    priority_queue<QPointF,  vector<QPointF>, gt> points; //cобытие точки
    priority_queue<event*, vector<event*>, gt> events; //событие круга
    arc* wave;//Начало двусвязного списка "береговой линии".
    QMultiMap<QPointF,properties> towers;//Характеристики вышек.
public:
    Fortune(){wave = nullptr;}
    void loadFromFile(QWidget* w);//Загружает данные из файла.
    void addPoint(QPointF& p);//Добавляет точку для расчета алгоритма.
    void startAlgorithm(QGraphicsScene* scene);
    void processPoint();
    void processEvent();
    void frontInsert(QPointF&  p);

    bool circle(QPointF& a, QPointF& b, QPointF& c, double *x, QPointF *o);//Находит крайнюю левую точку окружности
    void checkCircleEvent(arc *i, double x0);//Проверяет событие круга для i арки

    bool intersect(QPointF& p, arc *i, QPointF* result = 0);//Пересекает ли парабола с фокусом p арку i.
    QPointF intersection(QPointF& p0, QPointF& p1, double l);//Возвращает точку пересечения 2 парабол.
    void clipping(QPointF& a, QPointF& b);//Отсекает отрезки, вышедшие за область.

    void finishEdges();
    void finishCircuit();
    void addSideOfRect(QVector<QPointF>& A, QVector<QPointF>& B, QPointF& S, QPointF& F);//Добавляет граничные области в диаграму.
    void printOutput(QGraphicsScene* scene);
    void loadToFile(QString name);
    double function(double a1, double b1, double a2, double b2, double p1, double p2, double b);
};

#endif // FORTUNE_H
