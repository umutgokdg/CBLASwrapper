// BaseClass.h
#ifndef BASECLASS_H
#define BASECLASS_H

#include <stdio.h>

// Forward declaration of the BaseClass struct
typedef struct BaseClass BaseClass;

// Structure definition
struct BaseClass {
    void (*doSomething)(BaseClass*); // Function pointer for the method
};

// Function prototypes
void BaseClass_init(BaseClass* base);
void BaseClass_doSomething(BaseClass* self);
void BaseClass_cleanup(BaseClass* base);

#endif // BASECLASS_H
