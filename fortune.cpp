#include "fortune.h"

//Предикат для сравнения QPointF в дереве.
bool compare(arc* a, arc* b)
{
    return a->p.x() <= b->p.x();
}

void Fortune::startAlgorithm()
{
    wave.setPredicate(compare);
    points.push(QPointF(1,5));
    points.push(QPointF(3,2));
    points.push(QPointF(6,8));

    X1 = 6;
    Y1 = 8;

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

        //Удаление первой арки
        arc* a = e->a;
        wave.deleteNode(a);

        // Закончить вершины до и после a.
        if (a->s0) a->s0->finish(e->p);
        if (a->s1) a->s1->finish(e->p);

        // Перепроверка событий круга по обе стороны от p.
        arc* tmpArc = wave.findNode(wave.getRoot(),a)->getLeft()->getData();
        if (tmpArc != nullptr) checkCircleEvent(tmpArc, e->x);
        tmpArc = wave.findSuccsessor(a)->getData();
        if (tmpArc) checkCircleEvent(tmpArc, e->x);
    }
    delete e;
}

//TODO
void Fortune::frontInsert(QPointF& p)
{
    if (wave.getRoot() == nullptr) {
        wave.insertNode(new arc(p));
        return;
    }
    //Находит текущую арку(и) по высоте p.y (если они есть).
    for (arc* i = wave.getRoot()->getData(); i; i = wave.findSuccsessor(i)->getData()) {
        QPointF z, zz;
        if (intersect(p,i,&z)) {
            //Новая парабола пересекает арку i. Если нужно, дубрируется i.
            //if (wave.findSuccsessor(i) != nullptr && !intersect(p, wave.findSuccsessor(i)->getData(), &zz))
            wave.insertNode(new arc(i->p));

            wave.insertNode(new arc(p));
            //else i->next = new arc(i->p,i);
            wave.findSuccsessor(i)->getData()->s1 = i->s1;

            wave.insertNode(new arc(p));
            i = wave.findSuccsessor(i)->getData();

            // Добавление новых полуребер, свзанных с конечными точками i.
            wave.findNode(wave.getRoot(),i)->getLeft()->getData()->s1 = i->s0 = new seg(z);
            wave.findSuccsessor(i)->getData()->s0 = i->s1 = new seg(z);

            // Проверка на события круга вокруг новой арки
            checkCircleEvent(i, p.x());
            checkCircleEvent((wave.findNode(wave.getRoot(),i))->getLeft()->getData(), p.x());
            checkCircleEvent(wave.findSuccsessor(i)->getData(), p.x());
            return;
        }
    }

    //Cпециальный случай: если p никогда не пересечет арку,то нужно добавить p в список.
    arc* i = wave.findMax(wave.getRoot())->getData();

    wave.insertNode(new arc(p));
    //Вставка сегмента между p и i.
    QPointF start;
    start.setX(X0);
    start.setY((wave.findSuccsessor(i)->getData()->p.y() + i->p.y()) / 2);
    i->s1 = wave.findSuccsessor(i)->getData()->s0 = new seg(start);
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

    if ((wave.findNode(wave.getRoot(),i))->getLeft()->getData() != nullptr ||
            ( wave.findSuccsessor(i)->getData()) != nullptr)
        return;

    double x;
    QPointF o;

    //if (circle(i->prev->p, i->p, i->next->p, &x,&o) && x > x0)
    if(circle((wave.findNode(wave.getRoot(),i))->getLeft()->getData()->p, i->p,
              wave.findSuccsessor(i)->getData()->p, &x, &o) && x > x0)
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
    TreeNode<arc*>* node = wave.findNode(wave.getRoot(),i);//Узел, который проверяем на пересечение с соседями.
    if (node->getLeft() != nullptr) // Находит пересечение с предыдущей точкой.
        a = intersection(node->getLeft()->getData()->p, node->getData()->p, p.x()).y();
    TreeNode<arc*>* succsessor = wave.findSuccsessor(node->getData());
    if( succsessor!= nullptr)// Находит пересечение с последующей точкой.
        b = intersection(node->getData()->p, succsessor->getData()->p, p.x()).y();

    if ((node->getLeft() == nullptr || a <= p.y()) && (succsessor == nullptr || p.y() <= b)) {
        res->setY(p.y());

        //Подключение обратно в уравнение параболы.
        res->setX( (i->p.x()*i->p.x() + (i->p.y() - res->y())*(i->p.y() - res->y()) - p.x() * p.x())
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

void Fortune::finishEdges()
{
    // Advance the sweep line so no parabolas can cross the bounding box.
    double l = X1 + (X1-X0) + (Y1-Y0);

    // Extend each remaining segment to the new parabola intersections.
    for (arc* i = wave.getRoot()->getData(); wave.findSuccsessor(i)->getData();
         i = wave.findSuccsessor(i)->getData())
        if (i->s1)
            i->s1->finish(intersection(i->p, wave.findSuccsessor(i)->getData()->p , l*2));
}
