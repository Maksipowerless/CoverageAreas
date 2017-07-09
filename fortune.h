#ifndef FORTUNE_H
#define FORTUNE_H

#include"additinalstruct.h"
#include"tree.h"
#include"mypoint.h"
#include<QPointF>

class Fortune{
    double X0 = 0, X1 = 0, Y0 = 0, Y1 = 0;
    priority_queue<QPointF,  vector<QPointF>, gt> points; //cобытие точки
    priority_queue<event*, vector<event*>, gt> events; //событие круга
    Tree<arc*> wave;
public:
    void startAlgorithm();
    void processPoint();
    void processEvent();
    void frontInsert(QPointF&  p);

    bool circle(QPointF& a, QPointF& b, QPointF& c, double *x, QPointF *o);//Находит крайнюю левую точку окружности
    void checkCircleEvent(arc *i, double x0);//Проверяет событие круга для i арки

    bool intersect(QPointF& p, arc *i, QPointF* result = 0);//Пересекает ли парабола с фокусом p арку i.
    QPointF intersection(QPointF& p0, QPointF& p1, double l);//Возвращает точку пересечения 2 парабол.

    void finishEdges();
    void printOutput();
};

#endif // FORTUNE_H
