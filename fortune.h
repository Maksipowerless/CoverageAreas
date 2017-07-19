#ifndef FORTUNE_H
#define FORTUNE_H

#include"additinalstruct.h"
#include<QPointF>
#include<QGraphicsScene>
#include<QMap>
#include<cohensutherland.h>
#include<QMultiMap>
#include<QLinkedList>
#include<QFile>
#include<QTextStream>

//Cтруктура для хранения данных о вышке.
struct properties{
    int id;
    int tP;
    int az;
    properties()
        :id(0), tP(0),az(360){}
    properties(int i, int p, int a)
        : id(i),tP(p),az(a) {}
    properties operator=(const properties& p1){id = p1.id; tP = p1.tP; return *this;}
};

class CoveragesAzimuth;

class Fortune{
    double X0 = 0, X1 = 0, Y0 = 0, Y1 = 0;//Координаты ограничивающей области.
    priority_queue<QPointF,  vector<QPointF>, gt> points; //cобытие точки
    priority_queue<event*, vector<event*>, gt> events; //событие круга
    arc* wave;//Начало двусвязного списка "береговой линии".
    QMultiMap<QPointF,properties> towers;//Вышки и их характеристики.
    QVector<seg*> voronovDiagram;//Диаграмма Вороного.
public:
    friend CoveragesAzimuth;
    Fortune(){wave = nullptr;}

    //Ввод-вывод данных.
    void loadFromFile(QWidget* w);//Загружает данные о вышках из файла.
    void loadToFile(QString name);//Записывает диаграму Вороного в файл в формате kml.

    //Методы для алгоритма Форчуна.
    void startAlgorithm();
    void addPoint(QPointF& p);//Добавляет точку для расчета алгоритма.
    void processPoint();
    void processEvent();//Отработка события "точки".
    void frontInsert(QPointF&  p);//Вставка параболы в "береговую линию".
    bool circle(QPointF& a, QPointF& b, QPointF& c, double *x, QPointF *o);//Находит крайнюю левую точку окружности.
    void checkCircleEvent(arc *i, double x0);//Проверяет событие круга для i арки.
    bool intersect(QPointF& p, arc *i, QPointF* result = 0);//Пересекает ли парабола с фокусом p арку i.
    QPointF intersection(QPointF& p0, QPointF& p1, double l);//Возвращает точку пересечения 2 парабол.
    void finishEdges();//Завершает построение диаграмы.

    void clipping(QPointF& a, QPointF& b);//Отсекает отрезки, вышедшие за область x0,y0,x1,y1.
    double functionOfPower(double a1, double b1, double a2, double b2, double p1, double p2, double b);//Функция мощности.
};

#endif // FORTUNE_H
