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

bool operator<(const properties& p1, const properties& p2)
{
    return p1.id < p2.id;
}

bool operator>(const properties& p1, const properties& p2)
{
    return p1.id > p2.id;
}

//Загрузка данных о вышках из файла.
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
        double y = var.toDouble();//Широта

        var = strList[i+2];
        double x = var.toDouble();//Долгота

        var = strList[i+3];
        int p = var.toInt();

        towers.insert(QPointF(x,y), properties(id,p));

        i+=4;
    }
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

void Fortune::startAlgorithm(QGraphicsScene* scene)
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
        //scene->addEllipse(x-3,y-3,1,1,QPen(Qt::black), QBrush(Qt::red));
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
    //finishCircuit();
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

//Отсечение отрезков за прямоугольной областью.
void Fortune::clipping(QPointF &A, QPointF &B)
{
    rect* r = new rect(X0,Y0,X1,Y1);
    point* a = new point(A.x(), A.y());
    point* b = new point(B.x(), B.y());
    cohenSutherland(r, a, b);

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
    // Advance the sweep line so no parabolas can cross the bounding box.
    double l = X1 + (X1-X0) + (Y1-Y0);
    // Extend each remaining segment to the new parabola intersections.
    for (arc *i = wave; i->next; i = i->next)
        if (i->s1)
            i->s1->finish(intersection(i->p, i->next->p, l*2));

    //Отсечение отрезков за прямоугольной областью.
    vector<seg*>::iterator i;
    for (i = segmentsOfDiagramVoronoi.begin(); i != segmentsOfDiagramVoronoi.end(); i++)
        clipping((*i)->start,(*i)->end);
}

//TODO
void Fortune::finishCircuit()
{
    QPointF rightUp(-100,-100);//Правая верхняя вышка.
    QPointF rightDown(-100,-100);//Правая нижняя вышка.
    QPointF rightDownSecond(-100,-100);//Правая нижняя вышка.
    QPointF leftDown(-100,-100);//Левая нижняя вышка.

    QVector<QPointF> uper;
    QVector<QPointF> uperTowers;
    QVector<QPointF> tmleft;
    QVector<QPointF> tmleftTowers;
    QVector<QPointF> tmright;
    QVector<QPointF> tmrightTowers;
    QVector<QPointF> lower;
    QVector<QPointF> lowerTowers;

    //Запись граничных точек.
    vector<seg*>::iterator i;
    for (i = segmentsOfDiagramVoronoi.begin(); i != segmentsOfDiagramVoronoi.end(); i++)
    {
        QPointF p0 = (*i)->start;
        QPointF p1 = (*i)->end;

        if(p0 == p1)
        {
            segmentsOfDiagramVoronoi.erase(i);
            i--;
        }
        else
        {
            double eps = numeric_limits<double>::epsilon( );
            //eps = 0.01;
            //Запись граничных точек и вышек, которые рядом с границей.
            bool by0p0 = abs(p0.y() - Y0) < eps;
            bool by0p1 = abs(p1.y() - Y0) < eps;

            bool bx1p0 = abs(p0.x() - X1) < eps;
            bool bx1p1 = abs(p1.x() - X1) < eps;

            bool by1p0 = abs(p0.y() - Y1) < eps;
            bool by1p1 = abs(p1.y() - Y1) < eps;

            bool bx0p0 = abs(p0.x() - X0) < eps;
            bool bx0p1 = abs(p1.x() - X0) < eps;

            //Верхняя граница(Y0).
            if(by0p0 || by0p1)
            {
                if(by0p0)
                    uper.push_back(p0);
                else
                    uper.push_back(p1);

                if((*i)->tow[0].x() < (*i)->tow[1].x())
                {
                    uperTowers.push_back((*i)->tow[0]);
                    if((*i)->tow[1].x() > rightUp.x())
                        rightUp = (*i)->tow[1];
                }
                else
                {
                    uperTowers.push_back((*i)->tow[1]);\
                    if((*i)->tow[0].x() > rightUp.x())
                        rightUp = (*i)->tow[0];
                }
            }

            //Правая граница (X1).
            if(bx1p0 ||  bx1p1)
            {
                if(bx1p0)
                    tmright.push_back(p0);
                else
                    tmright.push_back(p1);

                if((*i)->tow[0].y() < (*i)->tow[1].y())
                {
                    tmrightTowers.push_back((*i)->tow[0]);
                    if((*i)->tow[1].y() > rightDown.y())
                        rightDown = (*i)->tow[1];
                }
                else
                {
                    tmrightTowers.push_back((*i)->tow[1]);\
                    if((*i)->tow[0].y() > rightDown.y())
                        rightDown = (*i)->tow[0];
                }
            }

            //Нижняя граница (Y1).
            if(by1p0 ||  by1p1)
            {
                if(by1p0)
                    lower.push_back(p0);
                else
                    lower.push_back(p1);

                if((*i)->tow[0].x() < (*i)->tow[1].x())
                {
                    lowerTowers.push_back((*i)->tow[0]);
                    if((*i)->tow[1].x() > rightDownSecond.x())
                        rightDownSecond = (*i)->tow[1];
                }
                else
                {
                    lowerTowers.push_back((*i)->tow[1]);
                    if((*i)->tow[0].x() > rightDownSecond.x())
                        rightDownSecond = (*i)->tow[0];
                }
            }


            //Левая граница (X1).
            if(bx0p0 ||  bx0p1)
            {
                if(bx0p0)
                    tmleft.push_back(p0);
                else
                    tmleft.push_back(p1);

                if((*i)->tow[0].y() < (*i)->tow[1].y())
                {
                    tmleftTowers.push_back((*i)->tow[0]);
                    if((*i)->tow[1].y() > leftDown.y())
                        leftDown = (*i)->tow[1];
                }
                else
                {
                    tmleftTowers.push_back((*i)->tow[1]);\
                    if((*i)->tow[0].y() > leftDown.y())
                        leftDown = (*i)->tow[0];
                }
            }

        }
    }

    qSort(uper);
    uperTowers.push_back(rightUp);
    qSort(uperTowers);

    qSort(tmright);
    tmrightTowers.push_back(rightDown);
    qSort(tmrightTowers.begin(),tmrightTowers.end(),compareY());

    qSort(lower);
    lowerTowers.push_back(rightDownSecond);
    qSort(lowerTowers);

    qSort(tmleft);
    tmleftTowers.push_back(leftDown);
    qSort(tmleftTowers.begin(),tmleftTowers.end(),compareY());

    //Построение граничных ребер.
    QPointF start(X0,Y0);
    QPointF finish(X1,Y0);
    addSideOfRect(uper, uperTowers, start, finish);

    start = QPointF(X1,Y0);
    finish = QPointF(X1,Y1);
    addSideOfRect(tmright, tmrightTowers, start, finish);

    start = QPointF(X0,Y1);
    finish = QPointF(X1,Y1);
    addSideOfRect(lower, lowerTowers, start, finish);

    start = QPointF(X0,Y0);
    finish = QPointF(X0,Y1);
    addSideOfRect(tmleft, tmleftTowers, start, finish);
}

void Fortune::addSideOfRect(QVector<QPointF>& vertex, QVector<QPointF>& T, QPointF& S, QPointF& F)
{
    QPointF temp;
    int count = vertex.size();
    //Если границу пересекают ребра диаграмы Вороного.
    if(count){
        for(int i = 0; i< count + 1; i++)
        {
            if(i == 0)
            {
                temp = vertex.takeFirst();
                QPointF p = S;
                seg* s = new seg(p);
                p = T.takeFirst();
                s->tow.push_back(p);
                s->end = temp;
                s->done = true;
            }
            else if(i == count)
            {
                seg* s = new seg(temp);
                QPointF p = T.takeFirst();
                s->tow.push_back(p);
                s->end = F;
                s->done = true;
            }
            else
            {
                seg* s = new seg(temp);
                QPointF p = T.takeFirst();
                s->tow.push_back(p);
                temp = vertex.takeFirst();
                s->end = temp;
                s->done = true;
            }
        }
    }
    //В противном случае добавляем сегмент - само ребро ограничивающей области.
    else
    {
        QList<QPointF> lst = towers.uniqueKeys();
        double lenth = numeric_limits<double>::infinity();
        QPointF closeTower;
        if(S.x() == F.x())
        {
            for(auto i = lst.begin(); i!=lst.end(); i++)
            {
                if(abs(S.x() - (*i).x()) < lenth)
                {
                    closeTower = (*i);
                    lenth = abs(S.x() - (*i).x());
                }
            }
        }
        else
        {
            for(auto i = lst.begin(); i!=lst.end(); i++)
            {
                if(abs(S.y() - (*i).y()) < lenth)
                {
                    closeTower = (*i);
                    lenth = abs(S.y() - (*i).y());
                }
            }

        }
        seg* s = new seg(S);
        s->end = F;
        s->done = true;
        s->tow.push_back(closeTower);
    }
}

void Fortune::printOutput(QGraphicsScene* scene)
{
    // scene->addLine(X0,Y0,X0,Y1);
    // scene->addLine(X0,Y0,X1,Y0);
    // scene->addLine(X1,Y0,X1,Y1);
    //  scene->addLine(X0,Y1,X1,Y1);

    //vector<seg*> a = segmentsOfDiagramVoronoi;

    double x1 =8;
    double y1 = 6;
    double x2 = 10;
    double y2 = 4;
    double p1 = 3;

    double p2 = 2;
    //double X = 5.313708498984761;
    //double Y = 10;

    QFile* file;
    QString str;
    QVariant var;
    for(double Y =-2; Y <19; Y+=0.5)
    {
        for(double X=0; X<10; X+=0.0000001)
        {
            double power1 = p1/((x1-X)*(x1-X) + (y1-Y)*(y1-Y));
            double power2 = p2/((x2-X)*(x2-X) + (y2-Y)*(y2-Y));
            if(abs(power1 - power2) < 0.0000001)
            {
                var = X;
                str+= var.toString();
                str+="; ";
                var = Y;
                str+=var.toString();
                str+="\n";
                X = 100;
            }
        }
    }

    file = new QFile("/home/maxfromperek/QtProjects/CoverageAreas/points.kml");
    file->open(QIODevice::WriteOnly);
    QTextStream so(file);
    so << str;
    file->close();

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
    vector<seg*>::iterator i;
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

double Fortune::function(double a1, double b1, double a2, double b2, double p1, double p2, double b)
{
    double A = p1-p2;
    double B = 2*(p2*a1 - p1*a2);
    double C = p1*(a2*a2 + b*b -2*b*b2 + b2*b2) - p2*(a1*a1 + b*b - 2*b*b1 + b1*b1);

    return (-B -sqrt(B*B - 4*A*C))/2*A;
}

/*
 набор для бага(точка 105,2)
11422;2;105;87;60
11423;105;2;64;200
11427;202;105;87;60
11428;242;145;87;60
11429;42;205;87;60
11430;321;312;87;60
11431;424;11;87;60
11433;22;505;87;60
11434;402;325;87;60
11438;252;445;87;60
11439;221;55;87;60
11440;62;341;87;60
11441;452;3*/

/*
 *  vector<seg*>::iterator it;
    for (it = segmentsOfDiagramVoronoi.begin(); it != segmentsOfDiagramVoronoi.end(); it++)
    {
        if((*it)->tow.size() == 2)
        {
            double a1 = (*it)->tow[0].x();
            double b1 = (*it)->tow[0].y();
            int p1 = towers.value((*it)->tow[0]).tP;

            double a2 = (*it)->tow[1].x();
            double b2 = (*it)->tow[1].y();
            int p2 = towers.value((*it)->tow[1]).tP;



            (*it)->start.setX(function(a1,b1,a2,b2,p1,p2,(*it)->start.y()));
            (*it)->end.setX(function(a1,b1,a2,b2,p1,p2,(*it)->end.y()));
        }

        //if((*i)->tow.contains(QPointF(221,55)))
        // scene->addLine(p0.x(), p0.y(), p1.x(), p1.y());

    }
*/
