#include <stdint.h>
#include <stdio.h>
extern "C" {

    // ������ ������� ��� ��������
    __declspec(dllexport) void hello_world() {
        printf("Hello, world from C++!");
    }

}
