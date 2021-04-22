int factorial(int x) {
    if (x == 0) {
        return 1;
    }
    return x * factorial(x - 1);
}

float e(int x) {
    float e = 0;
    for (int i = 0; i <= x; ++i) {
        e += 1. / factorial(i); 
    }
    return e;
}

float square(float a, float b) {
    return 0.5 * a * b;
}



