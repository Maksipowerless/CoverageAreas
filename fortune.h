#ifndef FORTUNE_H
#define FORTUNE_H

#include"additinalstruct.h"
#include"tree.h"
#include"mypoint.h"
#include<QPointF>
#include<QGraphicsScene>
#include<QMap>
#include<cohensutherland.h>

//Cтруктура для хранения данных о вышке.
struct coord{
    double tX;
    double tY;
    int tP;
    coord(double x, double y, int p)
        : tX(x), tY(y), tP(p) {}
};

class Fortune{
    double X0 = 0, X1 = 0, Y0 = 0, Y1 = 0;//Координаты ограничивающей области.
    priority_queue<QPointF,  vector<QPointF>, gt> points; //cобытие точки
    priority_queue<event*, vector<event*>, gt> events; //событие круга
    QMap<int,coord> towerProperties;//Характеристики вышки.
    arc* wave;//Начало двусвязного списка "береговой линии".
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
    void printOutput(QGraphicsScene* scene);
};

#endif // FORTUNE_H
