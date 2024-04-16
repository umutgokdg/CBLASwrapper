#include <iostream>
#include <string>
extern "C" {
    #include "BaseClass.h"
}

class WrapperClass {
private:
    BaseClass base; // Composition instead of inheritance

public:
    WrapperClass() {
        BaseClass_init(&base); // Initialize the C struct
    }

    ~WrapperClass() {
        BaseClass_cleanup(&base); // Clean up the C struct
    }

    void doSomething() {
        base.doSomething(&base); // Call the C function
        doExtra(); // Additional C++ functionality
    }

    void doExtra() {
        std::cout << "WrapperClass doing something extra." << std::endl;
    }
};

int main() {
    WrapperClass wrappedObject;

    // Call doSomething which internally calls the BaseClass doSomething and doExtra
    wrappedObject.doSomething();

    // Call doExtra directly
    wrappedObject.doExtra();

    return 0;
}
