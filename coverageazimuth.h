#ifndef COVERAGEAZIMUTH_H
#define COVERAGEAZIMUTH_H

#include"fortune.h"
#include"QLineF"

bool operator<(const QPointF a, const QPointF b);

//Структура представления сектора.
struct sector{
    QLineF l1;
    QLineF l2;
    QPointF center;
    QPointF normal;
    int azimuth;
    sector(QLineF line1, QLineF line2, QPointF p, QPointF n, int az)
        :l1(line1), l2(line2), center(p), normal(n), azimuth(az){}
    sector()
        :l1(0,0,0,0), l2(0,0,0,0), center(0,0), normal(0,0), azimuth(0){}
    QLineF& findLine(QLineF line){if(l1 == line) return l1; else return l2;}
};

/*В классе описаны методы поиска покрытий базовых станций с учетом азимута.*/
class CoveragesAzimuth{
     QMultiMap<QPointF,properties> towers;//Вышки и их характеристики.
     QMultiMap<QPointF,sector> towersSector;//Вышки с секторами обзора.
     QVector<seg*> voronovDiagram;
     double lenthOfSector;//Начальная длина ребра сектора.
public:
    CoveragesAzimuth(Fortune& f);
    void startAlgorythm();
    sector createSector(QPointF p, int azimuth);//Делает сектор из данной точки и азимута с длиной lnthOfSecor;
    void raysIntersection(QPointF &t, QPointF& secondT, QLineF &ray, QLineF &vor, QLineF &r1, QLineF &r2, QPointF &normalSecond);//Находит точку
    //пересечения луча сектора вышки t и лучей вышки secondT.

    void changeRay(QPointF t, QLineF& ray, QPointF& inter);//Изменяет длину луча в секторе.
    void printOutput(QGraphicsScene* scene);//Отображение секторов и диаграмы на сцене.
    bool coDirected(double ax, double ay, double bx, double by);//Проверка на 'сонаправленность'.

};

#endif // COVERAGEAZIMUTH_H
