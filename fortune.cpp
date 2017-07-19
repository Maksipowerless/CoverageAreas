#include "fortune.h"
#include <QFileDialog>
#include <QVariant>
#include <QFile>

//Предикат для сравнения QPointF в дереве.
bool compare(arc* a, arc* b)
{
    return a->p.x() < b->p.x();
}

//Оператор сравнения точек(нужен для QMultiMap)
bool operator<(const QPointF a, const QPointF b){return a.x() == b.x() ? a.y() < b.y() : a.x() < b.x();}

bool operator==(const properties& p1, const properties& p2)
{
    return p1.id == p2.id && p1.tP == p2.tP;
}

//Загрузка данных о вышках из файла.
void Fortune::loadFromFile(QWidget* w)
{
    QStringList strList;
    QString str;
    QVariant var;
    str = QFileDialog::getOpenFileName(w , "Enter filename");
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
        double y = var.toDouble();//Широта

        var = strList[i+2];
        double x = var.toDouble();//Долгота

        var = strList[i+3];
        int p = var.toInt();

        var = strList[i+4];
        int az = var.toInt();

        towers.insert(QPointF(x,y), properties(id,p,az));

        i+=4;
    }
}

void Fortune::loadToFile(QString name)
{
    QFile* file;
    QString str;
    str+="<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<kml xmlns=\"http://www.opengis.net/kml/2.2\">\n";

    //Добавление вышек.
    QVariant var;
    QList<QPointF> lst = towers.uniqueKeys();
    for(int i=0; i<lst.size(); i++)
    {
        str+= "<Placemark>\n";
        str+="<name>";

        QList<properties> pr = towers.values(lst[i]);
        for(int i=0; i < pr.size() ;i++)
        {
            var = pr[i].id;
            str+= var.toString();
            if(i!=pr.size()-1)
                str+=", ";
        }

        str+="</name>\n";
        str+="<Point>";
        str+="<coordinates>";
        var = lst[i].x();
        str+=var.toString();
        str+=", ";
        var = lst[i].y();
        str+= var.toString();
        str+="</coordinates>\n";
        str+="</Point>\n";
        str+="</Placemark>\n";
    }

    //Добавление границ.
    QVector<seg*>::iterator i;
    for (i = segmentsOfDiagramVoronoi.begin(); i != segmentsOfDiagramVoronoi.end(); i++)
    {
        str+= "<Placemark>\n";
        str+= "<LineString>\n";
        str+= "<coordinates>\n";
        var = (*i)->start.x();
        str+=var.toString();
        str+=", ";
        var = (*i)->start.y();
        str+=var.toString();
        str+="\n";

        var = (*i)->end.x();
        str+=var.toString();
        str+=", ";
        var = (*i)->end.y();
        str+=var.toString();
        str+="\n";
        str+="</coordinates>\n";
        str+="</LineString>\n";
    }

    str+="</kml>\n";
    file = new QFile(name);
    file->open(QIODevice::WriteOnly);
    QTextStream so(file);
    so << str;
    file->close();
}

void Fortune::addPoint(QPointF& p)
{
    p = QPointF(p.x(), p.y());
    points.push(p);
    if (p.x() < X0) X0 = p.x();
    if (p.y() < Y0) Y0 = p.y();
    if (p.x() > X1) X1 = p.x();
    if (p.y() > Y1) Y1 = p.y();
}

void Fortune::startAlgorithm()
{
    X0 = 1000;
    Y0 = 1000;
    //Добавление вышек(одинаковые координаты не дублируются)
    QList<QPointF> lst = towers.uniqueKeys();
    for(auto it = lst.begin(); it != lst.end(); it++)
    {
        double x = it->x();
        double y = it->y();
        QPointF p(x,y);
        addPoint(p);
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
        s->tow.push_back(e->a->prev->p);
        s->tow.push_back(e->a->next->p);

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

//Отсечение отрезков за прямоугольной областью.
void Fortune::clipping(QPointF &A, QPointF &B)
{
    rect* r = new rect(X0,Y0,X1,Y1);
    point* a = new point(A.x(), A.y());
    point* b = new point(B.x(), B.y());
    cohenSutherland(r, a, b);//Алгоритм Коэна — Сазерленда.

    A = QPointF(a->x, a->y);
    B = QPointF(b->x, b->y);

    delete r;
    delete a;
    delete b;
}

struct compareY{
    bool operator()(const QPointF& a, const QPointF& b){return a.y() < b.y();}
};

void Fortune::finishEdges()
{
    //Устанавливаем прямую L, чтобы параболы не пересекли ограничивающую рамку.
    double l = X1 + (X1-X0) + (Y1-Y0);
    //Завершение сегментов диаграмы.
    for (arc *i = wave; i->next; i = i->next)
        if (i->s1)
            i->s1->finish(intersection(i->p, i->next->p, l*2));

    //Отсечение отрезков за прямоугольной областью.
    QVector<seg*>::iterator i;
    for (i = segmentsOfDiagramVoronoi.begin(); i != segmentsOfDiagramVoronoi.end(); i++)
        clipping((*i)->start,(*i)->end);

    voronovDiagram = segmentsOfDiagramVoronoi;
}

double Fortune::functionOfPower(double a1, double b1, double a2, double b2, double p1, double p2, double b)
{
    double A = p1-p2;
    double B = 2*(p2*a1 - p1*a2);
    double C = p1*(a2*a2 + b*b -2*b*b2 + b2*b2) - p2*(a1*a1 + b*b - 2*b*b1 + b1*b1);

    return (-B -sqrt(B*B - 4*A*C))/2*A;
}
