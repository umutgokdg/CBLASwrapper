#include "../HFiles/c_library.h"
#include <stdlib.h>

Point* create_point(double x, double y) {
    Point* p = (Point*)malloc(sizeof(Point));
    if (p) {
        p->x = x;
        p->y = y;
    }
    return p;
}

void move_point(Point* p, double dx, double dy) {
    if (p) {
        p->x += dx;
        p->y += dy;
    }
}

void free_point(Point* p) {
    free(p);
}
