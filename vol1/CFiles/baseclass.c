// BaseClass.c
#include "BaseClass.h"

// Implementation of the doSomething function
void BaseClass_doSomething(BaseClass* self) {
    printf("BaseClass doing something.\n");
}

// Initialization function for BaseClass
void BaseClass_init(BaseClass* base) {
    base->doSomething = BaseClass_doSomething;
}

// Cleanup function for BaseClass
void BaseClass_cleanup(BaseClass* base) {
    // Cleanup resources here if needed
}
