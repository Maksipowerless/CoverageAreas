#ifndef MYPOINT_H
#define MYPOINT_H
//Переопределение класса точки, т.к. в QPoint нет оператора < >

class MyPoint{
    float m_x;
    float m_y;
public:
    MyPoint(float x, float y){m_x = x; m_y = y;}
    MyPoint(){m_x = 0; m_y = 0;}
    float x(){return m_x;}
    float y(){return m_y;}
    bool operator<(MyPoint& p){return this->x() < p.x();}
    bool operator>(MyPoint& p){return this->x() > p.x();}
    bool operator==(MyPoint& p){return this->x() == p.x() && this->y() == p.y();}
    MyPoint operator=(MyPoint& p){this->setX(p.x()), this->setY(p.y()); return *this;}
    void setX(double x){m_x=x;}
    void setY(double y){m_y=y;}
};

#endif // MYPOINT_H
