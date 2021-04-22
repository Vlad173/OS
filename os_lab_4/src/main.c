/* Епанешников В.С. М80-206Б-19
Родительский процесс создаёт дочерний процесс. Первой строкой пользователь пишет в консоль имя файла, которое будет передано
при создании дочернего процесса. Родительский процесс передает команды пользователя через pipe1, который связан с стандартным 
входным потоком дочернего процесса. Результаты своей работы дочерний процесс пишет в созданный им файл.
Вариант 1
Пользователь вводит команды вида: «число число число<endline>». Далее эти числа передаются от родительского процесса в дочерний.
Дочерний процесс считает их сумму и выводит её в файл. Числа имеют тип int.
*/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <ctype.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <semaphore.h>

#define STDIN 0
#define STDOUT 1
#define MIN_CAP 4

char* read_string(int fd) {
    size_t str_size = 0;
    size_t cap = MIN_CAP;
    char *str = (char*) malloc(sizeof(char) * cap);
    if (str == NULL) {
        perror("Malloc error");
        exit(-1);
    }
    char c;
    while (read(fd, &c, sizeof(char)) == 1) {
        if (c == '\n') {
            break;
        }
        str[(str_size)++] = c;
        if (str_size == cap) {
            str = (char*) realloc(str, sizeof(char) * cap * 3 / 2);
            cap = cap * 3 / 2;
            if (str == NULL) {
                perror("Realloc error");
                exit(-2);
            }
        }
    }
    str[str_size] = '\0';
    return str;
}

int str_length(char *str) {
    int length = 0;
    for (int i = 0; str[i] != '\0'; ++i) {
        ++length;
    }
    return length;
}

char* int_to_string(int sum) {
    int size = 0;
    int temp_sum = sum;
    while (temp_sum != 0) {
        temp_sum /= 10;
        ++size;
    }

    bool is_positive = true;
    if (sum < 0) {
        is_positive = false;
        sum *= -1;
        size++;
    }

    char* str = (char*) malloc(sizeof(char) * (size + 1));
    str[size] = '\0';
    for (int i = size - 1; i >= 0; --i) {
        if (!is_positive && i == 0) {
            str[0] = '-';
        } else {
            str[i] = sum % 10 + '0';
            sum /= 10;
        }
    }

    return str;
}

void child_work(char str[], char path[]) {
    mode_t mode = S_IRUSR | S_IWUSR;
	int flags = O_WRONLY | O_CREAT | O_TRUNC;
    int file = open(path, flags, mode);
    if (file == -1) {
        perror("Open error");
        exit(7);
    }  

    int sum = 0, number = 0, is_positive = 1;
    for (size_t i = 0; str[i] != '\0'; ++i) {
        if (str[i] == '-') {
            is_positive = -1;
            continue;
        }
        if (isdigit(str[i])) {
            number = number * 10 + str[i] - '0';
        }
        else if (str[i] == ' ') {
            sum += number * is_positive;
            number = 0;
            is_positive = 1;
        }
        
    }
    sum += number * is_positive;
    char* answer = int_to_string(sum);
    int answer_size = str_length(answer);
    
    printf("Child: ");
    fflush(stdout);
    answer[answer_size] = '\n';
    write(STDOUT, answer, sizeof(char) * answer_size + 1);

    write(file, answer, sizeof(char) * answer_size);

    close(file); 
    free(answer);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: ./a.out <filename>\n");
        exit(1);
    }

    char *file = argv[1];
    sem_t *sem = sem_open("semaphore", O_CREAT | O_EXCL, 0777, 0);
    if (sem == SEM_FAILED) {
        perror("Sem error");
        exit(10);
    }
    if (sem_unlink("semaphore") == -1) {
        perror("Sem_unlink error");
        exit(11);
    }
    
    int id = fork();
    
    if (id == -1) { 
        perror("Can\'t fork child");
        exit(2);
    } else if (id > 0) {
        int input = open("input.txt", O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
        if (input < 0) {
            perror("Open error");
            exit(2);
        }

        struct stat buff;
        if (fstat(input, &buff) < 0) {
            perror("Stat error");
            exit(3);
        }

        char* str = read_string(STDIN);
        size_t str_size = str_length(str);
        if (write(input, str, sizeof(char) * str_size) != str_size) {
            perror("write error");
            exit(4);
        }
        close(input);


        free(str);
        sem_post(sem);
        
        int child_status;
        if (wait(&child_status) == -1) {
            perror("wait error");
            exit(5);
        }

        int exit_code = WEXITSTATUS(child_status);
        if (!exit_code) {
            printf("Parent: The child process exited normally with exit code %d\n", exit_code);
        } else {
            printf("Parent: The child process exited abnormally with exit code %d\n", exit_code);
        }

        if (remove("input.txt") == -1) {
            perror("Remove error");
            exit(111);
        }
    
        printf("Parent: exit 0\n");
    }
    else {
        int fd = open("input.txt", O_RDWR);
        if (fd < 0) {
            perror("Open error");
            exit(5);
        }
        
        sem_wait(sem);
        struct stat buff;
        if (fstat(fd, &buff) < 0) {
            perror("Stat error");
            exit(6);
        }
        
        char *str = mmap(NULL, buff.st_size, PROT_READ, MAP_SHARED, fd, 0);
        if (str == MAP_FAILED) {
            perror("Mmap error");
            exit(7);
        }
        close(fd);

        child_work(str, file);

        if (munmap(str, buff.st_size) != 0) {
            perror("Munmap error");
            exit(8);
        }

        printf("Child: exit 0\n");
    }
    
    return 0;
}