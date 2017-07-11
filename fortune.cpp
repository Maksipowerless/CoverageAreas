#include "fortune.h"
#include <QFileDialog>
#include <QVariant>
#include <QFile>

//Предикат для сравнения QPointF в дереве.
bool compare(arc* a, arc* b)
{
    return a->p.x() < b->p.x();
}

void Fortune::loadFromFile(QWidget* w)
{
    QStringList strList;
    QString str;
    QVariant var;
    str = QFileDialog::getOpenFileName(w , "Enter filename");
    //str = "/home/maxfromperek/QtProjects/sf.csv";
    QFile file(str);
    if (file.open(QFile::ReadOnly))
    {
        str = file.readAll();
    }
    else
    {
        loadFromFile(w);
    }

    //Удаление символов, пока не встретиться цифра ('49' - 1; '57' - 9).
    for(int i=0; i< str.size(); i++)
    {
        if(str[i] >= 49 && str[i] <= 57)
        {
            break;
        }
        else
        {
            str.remove(i,1);
            i--;
        }
    }

    strList = str.split(QRegExp(";|\r\n|\n|\n\n"));
    //Запись характеристик в map.
    for(int i=0; i < strList.size()-1; i++)
    {
        var = strList[i];
        int id = var.toInt();

        var = strList[i+1];
        double x = var.toDouble();

        var = strList[i+2];
        double y = var.toDouble();

        var = strList[i+3];
        int p = var.toInt();

        towerProperties.insert(id, coord(x,y,p));

        i+=4;
    }
}

void Fortune::addPoint(QPointF& p)
{
    points.push(p);
    if (p.x() < X0) X0 = p.x();
    if (p.y() < Y0) Y0 = p.y();
    if (p.x() > X1) X1 = p.x();
    if (p.y() > Y1) Y1 = p.y();
}

void Fortune::startAlgorithm(QGraphicsScene* scene)
{
    //Добавление вышек.
    for(auto it = towerProperties.begin(); it != towerProperties.end(); it++)
    {
        double x = it->tX;
        double y = it->tY;
        QPointF p(x,y);
        addPoint(p);
        scene->addEllipse(x-3,y-3,1,1,QPen(Qt::black), QBrush(Qt::red));
    }

    //Добавление поле сетки.
    double dx = (X1-X0+1)/5.0, dy = (Y1-Y0+1)/5.0;
    X0 -= dx;  X1 += dx;  Y0 -= dy;  Y1 += dy;

    while (!points.empty())
        if (!events.empty() && events.top()->x <= points.top().x())
            processEvent();
        else
            processPoint();

    while (!events.empty())
        processEvent();

    finishEdges();
}

void Fortune::processPoint()
{
    //Берем верхнюю точку из очереди.
    QPointF p = points.top();
    points.pop();

    // Добавляем очередную дугу в "береговую линию".
    frontInsert(p);
}

void Fortune::processEvent()
{
    //Получаем следующее событие из очереди.
    event* e = events.top();
    events.pop();

    if (e->valid) {
        // Начинаем новую вершину.
        seg* s = new seg(e->p);

        //Удаление первой дуги.
        arc* a = e->a;
        if (a->prev) {
            a->prev->next = a->next;
            a->prev->s1 = s;
        }
        if (a->next) {
            a->next->prev = a->prev;
            a->next->s0 = s;
        }

        // Закончить вершины до и после a.
        if (a->s0) a->s0->finish(e->p);
        if (a->s1) a->s1->finish(e->p);

        // Перепроверка событий круга по обе стороны от p.
        if (a->prev) checkCircleEvent(a->prev, e->x);
        if (a->next) checkCircleEvent(a->next, e->x);
    }
    delete e;
}

void Fortune::frontInsert(QPointF& p)
{
    if (!wave) {
        wave = new arc(p);
        return;
    }

    //Находит текущую арку(и) по высоте p.y (если они есть).
    for (arc *i = wave; i; i = i->next) {
        QPointF z, zz;
        if (intersect(p,i,&z)) {
            //Новая парабола пересекает арку i. Если нужно, дубрируется i.
            if (i->next && !intersect(p,i->next, &zz)) {
                i->next->prev = new arc(i->p,i,i->next);
                i->next = i->next->prev;
            }
            else i->next = new arc(i->p,i);
            i->next->s1 = i->s1;

            //Добавление p между i и i->next,
            i->next->prev = new arc(p,i,i->next);
            i->next = i->next->prev;

            i = i->next;

            // Добавлчем сегменты в конечные точки.
            i->prev->s1 = i->s0 = new seg(z,i->prev->p,i->p);
            i->next->s0 = i->s1 = new seg(z,i->p, i->next->p);

            // Проверка новых событий круга.
            checkCircleEvent(i, p.x());
            checkCircleEvent(i->prev, p.x());
            checkCircleEvent(i->next, p.x());

            return;
        }
    }

    //Cпециальный случай: если p никогда не пересечет арку,то нужно добавить p в список.
    arc *i;
    for (i = wave; i->next; i=i->next) ; // Последний узел

    i->next = new arc(p,i);
    // Вставка сегмента между p и i.
    QPointF start;
    start.setX(X0);
    start.setY((i->next->p.y() + i->p.y()) / 2);
    i->s1 = i->next->s0 = new seg(start,i->p,i->next->p);
    int a = 0;

}
bool Fortune::circle(QPointF& a, QPointF& b, QPointF& c, double *x, QPointF *o)
{
    // Проверить что bc это правая сторона ab.
    if ((b.x() - a.x())*(c.y() - a.y()) - (c.x() - a.x())*(b.y() - a.y()) > 0)
        return false;

    double A = b.x() - a.x(),  B = b.y() - a.y(),
            C = c.x() - a.x(),  D = c.y() - a.y(),
            E = A*(a.x() + b.x()) + B*(a.y() + b.y()),
            F = C*(a.x() + c.x()) + D*(a.y() + c.y()),
            G = 2*(A*(c.y() - b.y()) - B*(c.x() - b.x()));

    if (G == 0) return false;  // Точки на одной прямой

    // Точка o - центр окружности.
    o->setX((D*E-B*F)/G);
    o->setY((A*F-C*E)/G);

    // o.x плюс радиус равно максимальной x координате.
    *x = o->x() + sqrt( pow(a.x() - o->x(), 2) + pow(a.y() - o->y(), 2));
    return true;
}

void Fortune::checkCircleEvent(arc *i, double x0)
{
    // Отменить любое старое событие
    if (i->e && i->e->x != x0)
        i->e->valid = false;
    i->e = NULL;

    if (!i->prev || !i->next)
        return;

    double x;
    QPointF o;

    if (circle(i->prev->p, i->p, i->next->p, &x,&o) && x > x0)
    {
        //Создане нового события.
        i->e = new event(x, o, i);
        events.push(i->e);
    }
}

bool Fortune::intersect(QPointF& p, arc *i, QPointF* res)
{
    if (i->p.x() == p.x()) return false;

    double a,b;
    if (i->prev)//Пересечение i->prev и i
        a = intersection(i->prev->p, i->p, p.x()).y();
    if (i->next) //Пересечение i->next и i
        b = intersection(i->p, i->next->p, p.x()).y();

    if ((!i->prev || a <= p.y()) && (!i->next || p.y() <= b)) {
        res->setY(p.y());

        // Plug it back into the parabola equation.
        res->setX((i->p.x()*i->p.x() + (i->p.y() - res->y())*(i->p.y() - res->y()) - p.x()*p.x())
                  / (2*i->p.x() - 2*p.x()));

        return true;
    }
    return false;
}

QPointF Fortune::intersection(QPointF& p0, QPointF& p1, double l)
{
    QPointF res, p = p0;
    if (p0.x() == p1.x())
        res.setY((p0.y() + p1.y()) / 2);
    else if (p1.x() == l)
        res.setY(p1.y());
    else if (p0.x() == l) {
        res.setY(p0.y());
        p = p1;
    } else {
        double z0 = 2*(p0.x() - l);
        double z1 = 2*(p1.x() - l);

        double a = 1/z0 - 1/z1;
        double b = -2*(p0.y()/z0 - p1.y()/z1);
        double c = (p0.y()*p0.y() + p0.x()*p0.x() - l*l)/z0
                - (p1.y()*p1.y() + p1.x()*p1.x() - l*l)/z1;

        res.setY(( -b - sqrt(b*b - 4*a*c) ) / (2*a));
    }
    res.setX((p.x()*p.x() + (p.y()-res.y())*(p.y() - res.y()) - l*l)/(2*p.x() - 2*l));
    return res;
}

void Fortune::clipping(QPointF &A, QPointF &B)
{
    rect* r = new rect(X0,Y0,X1,Y1);
    point* a = new point(A.x(), A.y());
    point* b = new point(B.x(), B.y());
    cohenSutherland(r, a, b);

    A = QPoint(a->x, a->y);
    B = QPoint(b->x, b->y);

    delete r;
    delete a;
    delete b;
}

void Fortune::finishEdges()
{
    // Advance the sweep line so no parabolas can cross the bounding box.
    double l = X1 + (X1-X0) + (Y1-Y0);
    // Extend each remaining segment to the new parabola intersections.
    for (arc *i = wave; i->next; i = i->next)
        if (i->s1)
            i->s1->finish(intersection(i->p, i->next->p, l*2));

    //Отсечение отрезков за прямоугольной областью.
    vector<seg*>::iterator i;
    for (i = output.begin(); i != output.end(); i++)
        clipping((*i)->start,(*i)->end);
}

void Fortune::printOutput(QGraphicsScene* scene)
{

    vector<seg*> a = output;
    scene->addLine(X0,Y0,X0,Y1);
    scene->addLine(X0,Y0,X1,Y0);
    scene->addLine(X1,Y0,X1,Y1);
    scene->addLine(X0,Y1,X1,Y1);

    vector<seg*>::iterator i;
    for (i = output.begin(); i != output.end(); i++) {
        QPointF p0 = (*i)->start;
        QPointF p1 = (*i)->end;
        scene->addLine(p0.x(), p0.y(), p1.x(), p1.y());
    }
}
