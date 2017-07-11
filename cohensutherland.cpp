#include "cohensutherland.h"

void cohenSutherland (const struct rect *r, struct point *a, struct point *b)
{
    int code_a, code_b, code; /* код концов отрезка */
    struct point *c; /* одна из точек */

    code_a = vcode(r, a);
    code_b = vcode(r, b);

    /* пока одна из точек отрезка вне прямоугольника */
    while (code_a | code_b) {

        /* выбираем точку c с ненулевым кодом */
        if (code_a) {
            code = code_a;
            c = a;
        } else {
            code = code_b;
            c = b;
        }

        /* если c левее r, то передвигаем c на прямую x = r->x_min
           если c правее r, то передвигаем c на прямую x = r->x_max */
        if (code & LEFT) {
            c->y += (a->y - b->y) * (r->x_min - c->x) / (a->x - b->x);
            c->x = r->x_min;
        } else if (code & RIGHT) {
            c->y += (a->y - b->y) * (r->x_max - c->x) / (a->x - b->x);
            c->x = r->x_max;
        }/* если c ниже r, то передвигаем c на прямую y = r->y_min
            если c выше r, то передвигаем c на прямую y = r->y_max */
        else if (code & BOT) {
            c->x += (a->x - b->x) * (r->y_min - c->y) / (a->y - b->y);
            c->y = r->y_min;
        } else if (code & TOP) {
            c->x += (a->x - b->x) * (r->y_max - c->y) / (a->y - b->y);
            c->y = r->y_max;
        }

        /* обновляем код */
        if (code == code_a) {
            a = c;
            code_a = vcode(r,a);
        } else {
            b = c;
            code_b = vcode(r,b);
        }
    }

}
