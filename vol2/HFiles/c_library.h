#ifndef C_LIBRARY_H
#define C_LIBRARY_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Point {
    double x, y;
} Point;

// Function to create a Point
Point* create_point(double x, double y);

// Function to move a Point by dx and dy
void move_point(Point* p, double dx, double dy);

// Function to free a Point
void free_point(Point* p);

#ifdef __cplusplus
}
#endif

#endif // C_LIBRARY_H

