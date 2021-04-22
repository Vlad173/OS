#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>

#define LIBNAME_FIRST "liblab1.so"
#define LIBNAME_SECOND "liblab2.so"

int main() {
    float (*e) (int) = NULL;
    float (*square) (float, float) = NULL;
    
    void *handle1 = dlopen(LIBNAME_FIRST, RTLD_LAZY);
    if (!handle1) {
        printf("%s\n", dlerror());
        exit(1);
    }

    void *handle2 = dlopen(LIBNAME_SECOND, RTLD_LAZY);
    if (!handle2) {
        printf("%s\n", dlerror());
        exit(2);
    }
    int lib = 1;
    int cmd = 0;
    int x = 0;
    float a = 0.f, b = 0.f;
    char *error;

    while (scanf("%d", &cmd) != EOF) {
        switch(cmd) {
        case 0:
            lib = (lib == 1) ? 2 : 1;
            break;
        case 1:
            if (lib == 1) {
                e = dlsym(handle1, "e");
            }
            else {
                e = dlsym(handle2, "e");
            }

            if ((error = dlerror()) != NULL) {
                printf("Dlsym error\n");
                exit(3);
            }

            scanf("%d", &x);
            printf("Result: %f\n", e(x));
            break;
        case 2:
            if (lib == 1) {
                square = dlsym(handle1, "square");
            }  
            else {
                square = dlsym(handle2, "square");
            }

            if ((error = dlerror()) != NULL) {
                printf("Dlsym error\n");
                exit(4);
            }
            
            scanf("%f %f", &a, &b);
            printf("Result: %f\n", square(a, b));
            break;
        }
    }

    if (dlclose(handle1) != 0) {
        perror("Dlclose error");
        exit(5);
    }
    if (dlclose(handle2) != 0) {
        perror("Dlclose error");
        exit(6);
    }

    return 0;
}
