#ifndef ADDITINALSTRUCT_H
#define ADDITINALSTRUCT_H

#include <iostream>
#include <queue>
#include <set>
#include <math.h>
#include<QPointF>
#include<QVector>

using namespace std;
struct seg;
struct arc;
static QVector<seg*> segmentsOfDiagramVoronoi;//Диаграма Вороного.

//События в алгоритме Форчуна.
struct event {
    double x;
    QPointF p;
    arc *a;
    bool valid;

    event(double xx, QPointF pp, arc *aa)
        : x(xx), p(pp), a(aa), valid(true) {}
};

//Парабола "береговой линии".
struct arc {
    QPointF p;
    arc *prev, *next;
    event *e;

    seg *s0, *s1;

    arc(QPointF& pp, arc *a=0, arc *b=0)
        : p(pp), prev(a), next(b), e(0), s0(0), s1(0) {}
};

//Ребра локуса(диаграмы Вороного).
struct seg {
    QPointF start, end;
    bool done;
    QVector<QPointF> tow;//Вышки, которые находятся рядом с этим ребром.

    seg(QPointF& p)
        : start(p), end(0,0), done(false)
    {
        segmentsOfDiagramVoronoi.push_back(this);}
    seg(QPointF& p, QPointF A, QPointF B)
        : start(p), end(0,0), done(false)
    {
        segmentsOfDiagramVoronoi.push_back(this); tow.push_back(A); tow.push_back(B);}
    void finish(QPointF p) { if (done) return; end = p; done = true; }
};

//Предиката для сортировки QPointF и событий event в очереди.
struct gt {
    bool operator()(QPointF& a, QPointF& b) {return a.x()==b.x() ? a.y() > b.y() : a.x() > b.x();}
    bool operator()(event *a, event *b) {return a->x>b->x;}
};

#endif // ADDITINALSTRUCT_H
