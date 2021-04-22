#include <stdio.h>

#include "./libs/libs.h"


int main() {
    int cmd = 0;
    int x = 0;
    float a = 0.f, b = 0.f;
    
    while (scanf("%d", &cmd) != EOF) {
        switch(cmd) {
        case 0:
            printf("This is not program 2\n");
            break;
        case 1:
            scanf("%d", &x);
            printf("Result: %f\n", e(x));
            break;
        case 2:
            scanf("%f %f", &a, &b);
            printf("Result: %f\n", square(a, b));
            break;
        }
    }
    return 0;
}


