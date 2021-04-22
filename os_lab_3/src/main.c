#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define SEC 1000000000

typedef long long int ll;

typedef struct {
    int **mat;
    size_t order;
} Matrix;

typedef struct {
    size_t begin;
    size_t end;
    Matrix *mat;
    ll *result;
} Args;


void m_init(Matrix *m, size_t size) {
    m->order = size;
    m->mat = (int**) malloc(sizeof(int*) * m->order);
    for (size_t i = 0; i < m->order; ++i) {
        m->mat[i] = (int*) malloc(sizeof(int) * m->order);
    }
}

void m_input(Matrix *m) {
    int value;
    scanf("%zd", &m->order);
    m->mat = (int**) malloc(sizeof(int*) * m->order);
    for (size_t i = 0; i < m->order; ++i) {
        m->mat[i] = (int*) malloc(sizeof(int) * m->order);
        for (size_t j = 0; j < m->order; ++j) {
            scanf("%d", &value);
            m->mat[i][j] = value;
        }
    }
}

void m_print(Matrix *m) {
    for (size_t i = 0; i < m->order; ++i) {
        for (size_t j = 0; j < m->order; ++j) {
            printf("%d ", m->mat[i][j]);
        }
        printf("\n");
    }
}

void m_delete(Matrix *m) {
    for (size_t i = 0; i < m->order; ++i) {
        free(m->mat[i]);
    }
    free(m->mat);
}

void getMatrix(Matrix *matrix, size_t row, size_t col, Matrix *newMatrix) {
    size_t offsetRow = 0;
    size_t offsetCol = 0; 

    for (size_t i = 0; i < newMatrix->order; ++i) {
        if (i == row) {
            offsetRow = 1;
        }

        offsetCol = 0;
        for (size_t j = 0; j < newMatrix->order; ++j) {
            if (j == col) {
                offsetCol = 1;
            }
            newMatrix->mat[i][j] = matrix->mat[i + offsetRow][j + offsetCol];
        }
    }
}

ll determinant(Matrix *m) {
    ll det = 0;
    int degree = 1;

    if (m->order == 1) {
        return m->mat[0][0];
    } 
    else if (m->order == 2) {
        return m->mat[0][0] * m->mat[1][1] - m->mat[0][1] * m->mat[1][0];
    } 
    else {
        Matrix newMatrix;
        m_init(&newMatrix, m->order - 1);
        for (size_t j = 0; j < m->order; ++j) {
            getMatrix(m, 0, j, &newMatrix);
            det += degree * m->mat[0][j] * determinant(&newMatrix);
            degree *= -1;
        }

        m_delete(&newMatrix);
    }

    return det;
}


void *_matrix(void *args) {
    Args *params = (Args*) args;
    Matrix newMatrix;
    m_init(&newMatrix, params->mat->order - 1);
    ll temp = 0;
    for (size_t i = params->begin; i < params->end; ++i) {
        getMatrix(params->mat, 0, i, &newMatrix);
        temp += pow(-1, i) * params->mat->mat[0][i] * determinant(&newMatrix);
    } 
    *params->result = temp;
    m_delete(&newMatrix);

    return NULL;
}

ll determiant_threads(Matrix *m, size_t threads_limit) {
    size_t quotient = m->order / threads_limit;
    size_t remainder = m->order % threads_limit;
    pthread_t threads[threads_limit];
    size_t it = 0;
    Args data[threads_limit];
    ll results[threads_limit];

    for (size_t i = 0; i < threads_limit; ++i) {
        data[i].result = &results[i];
        data[i].begin = it;
        data[i].mat = m;
        it += quotient;
        if (remainder > 0) {
            ++it;
            --remainder;
        }
        data[i].end = it;

        if (pthread_create(&threads[i], NULL, _matrix, &data[i]) != 0) {
            printf("error with thread creating occured\n");
            exit(-2);
        }
    }

    ll result = 0;
    for (size_t i = 0; i < threads_limit; ++i) {
        pthread_join(threads[i], NULL);
        result += results[i];
    }
    
    
    return result;
}

int main(int argc, char *argv[]) {
    if (argc != 2 || atoi(argv[1]) == 0) {
        printf("Bad arguments\n");
        exit(-1);
    }

    Matrix m;
    m_input(&m);

    
    size_t threads_limit = atoi(argv[1]);
    struct timespec start, end;
    if (m.order < threads_limit) {
        threads_limit = m.order;
    }
    timespec_get(&start, TIME_UTC);
    ll result = determiant_threads(&m, threads_limit);
    timespec_get(&end, TIME_UTC);

    printf("Time with %zu threads: %lf\nResult: %lld\n", threads_limit, ((end.tv_sec * SEC + end.tv_nsec) - 
                                                    (start.tv_sec * SEC + start.tv_nsec)) / (double) SEC, result);

    timespec_get(&start, TIME_UTC);
    result = determinant(&m);
    timespec_get(&end, TIME_UTC);


    printf("Time without threads: %lf\nResult: %lld\n", ((end.tv_sec * SEC + end.tv_nsec) - 
                                                 (start.tv_sec * SEC + start.tv_nsec)) / (double) SEC, result);

    m_delete(&m);
    return 0;
}