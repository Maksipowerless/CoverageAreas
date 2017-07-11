#ifndef COHENSUTHERLAND_H
#define COHENSUTHERLAND_H

#include<QPointF>

#define LEFT  1  /* двоичное 0001 */
#define RIGHT 2  /* двоичное 0010 */
#define BOT   4  /* двоичное 0100 */
#define TOP   8  /* двоичное 1000 */

void cohenSutherland(const struct rect *r, struct point *a, struct point *b);
/* точка */
struct point {
    double x, y;
    point(double xx, double yy)
        :x(xx), y(yy){}
};

/* прямоугольник */
struct rect {
    double x_min, y_min, x_max, y_max;
    rect(double x1, double y1, double x2, double y2)
        : x_min(x1), y_min(y1), x_max(x2), y_max(y2) {}
};

/* вычисление кода точки
   r : указатель на struct rect; p : указатель на struct point */
#define vcode(r, p) \
    ((((p)->x < (r)->x_min) ? LEFT : 0)  +  /* +1 если точка левее прямоугольника */ \
     (((p)->x > (r)->x_max) ? RIGHT : 0) +  /* +2 если точка правее прямоугольника */\
     (((p)->y < (r)->y_min) ? BOT : 0)   +  /* +4 если точка ниже прямоугольника */  \
     (((p)->y > (r)->y_max) ? TOP : 0))     /* +8 если точка выше прямоугольника */

/* если отрезок ab не пересекает прямоугольник r, функция возвращает -1;
   если отрезок ab пересекает прямоугольник r, функция возвращает 0 и отсекает
   те части отрезка, которые находятся вне прямоугольника */


#endif // COHENSUTHERLAND_H
