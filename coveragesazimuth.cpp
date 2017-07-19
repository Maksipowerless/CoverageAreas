#include"coverageazimuth.h"

CoveragesAzimuth::CoveragesAzimuth(Fortune &f)
{
    towers = f.towers;
    voronovDiagram = f.voronovDiagram;
    lenthOfSector = (f.X1 - f.X0 > f.Y1 - f.Y0) ? 2*(f.X1 - f.X0) : 2*(f.Y1 - f.Y0);
}

void CoveragesAzimuth::startAlgorythm()
{
    //Расчет секторов обзора вышек.
    QList<QPointF> lst = towers.uniqueKeys();
    for(auto it = lst.begin(); it != lst.end(); it++)
    {
        QPointF tower = *it;
        int az = towers.value(*it).az;
        towersSector.insert(tower, createSector(tower, az));
    }

    //Непосредственно сам алгоритм.
    for(auto it = voronovDiagram.begin(); it!= voronovDiagram.end(); it++)
    {
        QPointF towerA = (*it)->tow[0];
        QPointF towerB = (*it)->tow[1];

        QLineF voronovEdge = QLineF((*it)->start.x(), (*it)->start.y(), (*it)->end.x(), (*it)->end.y());
        QLineF lineA1 = towersSector.value(towerA).l1;
        QLineF lineA2 = towersSector.value(towerA).l2;
        QLineF lineB1 = towersSector.value(towerB).l1;
        QLineF lineB2 = towersSector.value(towerB).l2;
        QPointF normalA = towersSector.value(towerA).normal;
        QPointF normalB = towersSector.value(towerB).normal;

        //Рассматриваем соседние вышки(получили с помощью диаграммы Вороного).
        //Отсекаем лучи (рассматриваем каждый с каждым из соседних вышек).
        raysIntersection(towerA, towerB, lineA1, voronovEdge, lineB1, lineB2, normalB);
        raysIntersection(towerA, towerB, lineA2, voronovEdge, lineB1, lineB2, normalB);
        raysIntersection(towerB, towerA, lineB1, voronovEdge, lineA1, lineA2, normalA);
        raysIntersection(towerB, towerA, lineB2, voronovEdge, lineA1, lineA2, normalA);

    }
}

enum intersections {NO_INTER = 0,REAL = 1, IMAGINARY = 2};//Пересечение прямых: 0-не пересекаются, 1-пересекаются, 2-пересекаются на продолжениях.
void CoveragesAzimuth::raysIntersection(QPointF& t, QPointF& secondT, QLineF& ray, QLineF& vor, QLineF& r1, QLineF& r2, QPointF& normalSecond)
{
    //Точки пересечения луча с лучами другой вышки и диаграмой Вороного.
    QPointF intersectionVoronov;
    QPointF intersectionRay1;
    QPointF intersectionRay2;

    //Находим точки пересечения данного луча с 2 другими и ребром диаграммы Вороного. Узнаем их тип.
    int pointVoronov = ray.intersect(vor, &intersectionVoronov);
    int pointRay1 = ray.intersect(r1, &intersectionRay1);
    int pointRay2 = ray.intersect(r2, &intersectionRay2);

    QLineF tRay1(t.x(), t.y(), intersectionRay1.x(), intersectionRay1.y());//Отрезок от вышки t до точки пересечения с лучом ray1.
    QLineF tRay2(t.x(), t.y(), intersectionRay2.x(), intersectionRay2.y());//Отрезок от вышки t до точки пересечения с лучом ray2.
    QLineF tVor(t.x(), t.y(), intersectionVoronov.x(), intersectionVoronov.y());//Отрезок от вышки до точки пересечения с локусом.

    double lenthTRay1 = tRay1.length();
    double lenthTRay2 = tRay2.length();
    double lenthTVor = tVor.length();

    //Рассматриваем все возможные случаи пересечения.
    if(pointVoronov == REAL && pointRay1 == REAL && pointRay2 == IMAGINARY)
    //Проверяем 'сонаправленность' 2 векторов: первый - начало это точка пересечения с 1 лучом, конец - вышка t;
    //второй - начало точка лежащая на азимуте сектора вышки secondT, конец - вышка secondT.
    {
        double aX = intersectionRay1.x() - t.x();
        double aY = intersectionRay1.y() - t.y();
        double bX = normalSecond.x() - secondT.x();
        double bY = normalSecond.y() - secondT.y();
             if(coDirected(aX, aY, bX, bY))
                 changeRay(t,ray, intersectionRay1);
             else
                 changeRay(t,ray, intersectionVoronov);
    }
    //Проверяем 'сонаправленность' 2 векторов: первый - начало это точка пересечения с 2 лучом, конец - вышка t;
    //второй - начало точка лежащая на азимуте сектора вышки secondT, конец - вышка secondT.
    else if(pointVoronov == REAL && pointRay2 == REAL && pointRay1 == IMAGINARY)
    {
       double aX = intersectionRay2.x() - t.x();
       double aY = intersectionRay2.y() - t.y();
       double bX = normalSecond.x() - secondT.x();
       double bY = normalSecond.y() - secondT.y();
            if(coDirected(aX, aY, bX, bY))
                changeRay(t,ray, intersectionRay2);
            else
                changeRay(t,ray, intersectionVoronov);
    }
    else if(pointVoronov == REAL && pointRay2 == REAL && pointRay1 == REAL)
    //Берем точку, которая находится посередине.
    {
        if((lenthTRay1 < lenthTVor && lenthTVor < lenthTRay2) || ( lenthTRay2 < lenthTVor && lenthTVor < lenthTRay1))
            changeRay(t,ray, intersectionVoronov);
        else if((lenthTVor < lenthTRay1 && lenthTRay1 < lenthTRay2) || (lenthTRay2 < lenthTRay1 && lenthTRay1 < lenthTVor))
            changeRay(t,ray, intersectionRay1);
        else
            changeRay(t,ray, intersectionRay2);
    }
}

void CoveragesAzimuth::changeRay(QPointF t, QLineF& ray, QPointF& inter)
{
    sector s = towersSector.take(t);
    s.findLine(ray).setLine(s.center.x(), s.center.y(), inter.x(), inter.y());
    towersSector.insert(t,s);
}

sector CoveragesAzimuth::createSector(QPointF p, int azimuth)
{
    double r = lenthOfSector;
    double x = p.x() + r * sin((azimuth - 60) * M_PI/180);
    double y = p.y() - r * cos((azimuth - 60) * M_PI/180);

    QLineF l1(p.x(), p.y(), x, y);

    x = p.x() + r * sin((azimuth + 60) * M_PI/180);
    y = p.y() - r * cos((azimuth + 60) * M_PI/180);

    QLineF l2(p.x(), p.y(), x, y);

    x = p.x() + r/10 * sin(azimuth * M_PI/180);
    y = p.y() - r/10 * cos(azimuth * M_PI/180);

    QPointF normal(x,y);

    return sector(l1,l2,p, normal,azimuth);
}

void CoveragesAzimuth::printOutput(QGraphicsScene *scene)
{
    for(auto it = towersSector.begin(); it != towersSector.end(); it++)
    {
        scene->addLine((*it).l1,QPen(Qt::green));
        scene->addLine((*it).l2,QPen(Qt::green));
    }

    for(auto it = voronovDiagram.begin(); it != voronovDiagram.end(); it++)
    {
        QPointF s = (*it)->start;
        QPointF f = (*it)->end;
        scene->addLine(s.x(), s.y(), f.x(), f.y());

    }
}

bool CoveragesAzimuth::coDirected(double ax, double ay, double bx, double by)
{
    if(abs(ax/bx - ay/by) < numeric_limits<double>::epsilon( ))
    {
        return ax/bx + ay/by > 0;
    }
    else
    {
        double cx = ax + bx;
        double cy = ay + by;
        double lenthA = sqrt(ax*ax + ay*ay);
        double lenthB = sqrt(bx*bx + by*by);
        double lenthC = sqrt(cx*cx + cy*cy);
        if(lenthC < lenthA || lenthC < lenthB)
            return false;
        else return true;
    }
}

//Набор вышек для демонстрации алгоритма.

/*
11422;100;10;87;150
11423;50;120;64;150
11425;200;20;83;60
11426;175;160;182;315
11427;150;200;67;310
*/


/*
11422;100;10;87;200
11423;50;120;64;340
11425;200;20;83;100
11426;175;160;182;90
11427;150;200;67;58
*/


