#ifndef POINT_WRAPPER_HPP
#define POINT_WRAPPER_HPP

extern "C" {
    #include "../HFiles/c_library.h"
}
#include <memory>
#include <stdexcept>

class PointWrapper {
private:
    std::unique_ptr<Point, void (*)(Point*)> ptr;

public:
    PointWrapper(double x, double y) : ptr(create_point(x, y), free_point) {
        if (!ptr) {
            throw std::runtime_error("Failed to create Point object");
        }
    }

    void move(double dx, double dy) {
        move_point(ptr.get(), dx, dy);
    }

    double x() const { return ptr->x; }
    double y() const { return ptr->y; }
};

#endif // POINT_WRAPPER_HPP
