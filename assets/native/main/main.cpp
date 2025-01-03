#include <stdint.h>
#include <stdio.h>
extern "C" {

    // Пример функции для экспорта
    __declspec(dllexport) void hello_world() {
        printf("Hello, world from C++!");
    }

}
