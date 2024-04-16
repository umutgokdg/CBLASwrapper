#include <iostream>
extern "C" {
    #include "HFiles/c_library.h"
}
#include "HppFiles/point_wrapper.hpp"

int main() {
    try {
        PointWrapper p(3.0, 4.0);
        std::cout << "Initial Position: (" << p.x() << ", " << p.y() << ")" << std::endl;
        
        p.move(1.0, 1.0);
        std::cout << "After Moving: (" << p.x() << ", " << p.y() << ")" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    return 0;
}
