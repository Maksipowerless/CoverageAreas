#ifndef ADDITINALSTRUCT_H
#define ADDITINALSTRUCT_H

#include <iostream>
#include <queue>
#include <set>
#include <math.h>
#include<mypoint.h>
#include<fortune.h>
#include<QPointF>

using namespace std;
struct seg;
struct arc;
static vector<seg*> output;

//События в алгоритме.
struct event {
    double x;
    QPointF p;
    arc *a;
    bool valid;

    event(double xx, QPointF pp, arc *aa)
        : x(xx), p(pp), a(aa), valid(true) {}
};

//Параболы береговой линии.
struct arc {
    QPointF p;
      arc *prev, *next;
      event *e;

      seg *s0, *s1;

      arc(QPointF& pp, arc *a=0, arc *b=0)
       : p(pp), prev(a), next(b), e(0), s0(0), s1(0) {}
};

//Ребра локуса.
struct seg {
    QPointF start, end;
    bool done;
    QPointF vertexA;
    QPointF vertexB;

    seg(QPointF& p)
        : start(p), end(0,0), done(false)
    {output.push_back(this);}
    seg(QPointF& p, QPointF A, QPointF B)
        : start(p), end(0,0), done(false), vertexA(A), vertexB(B)
    {output.push_back(this);}
    void finish(QPointF p) { if (done) return; end = p; done = true; }
};

//Предиката для сортировки QPointF и событий event в очереди.
struct gt {
    bool operator()(QPointF& a, QPointF& b) {return a.x()==b.x() ? a.y() > b.y() : a.x() > b.x();}
    bool operator()(event *a, event *b) {return a->x>b->x;}
};

#endif // ADDITINALSTRUCT_H
