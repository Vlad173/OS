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
    if (str == NULL) {
        perror("Malloc error");
        exit(-20);
    }
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
    if (fflush(stdout) != 0) {
        perror("Fflush error");
        exit(24);
    }
    answer[answer_size] = '\n';
    if (write(STDOUT, answer, sizeof(char) * answer_size + 1) != answer_size + 1) {
        perror("Write error");
        exit(25);
    }

    if (write(file, answer, sizeof(char) * answer_size) != answer_size) {
        perror("Write error");
        exit(26);
    }

    if (close(file) == -1) {
        perror("Close error");
        exit(27);
    }
    free(answer);
}

int main() {
    int fd[2];
    if (pipe(fd) < 0){
        perror("Can\'t create pipe");
        exit(-3); 
    } 

    char *path = read_string(STDIN);
    int id = fork();
    
    if (id == -1) { 
        perror("Can\'t fork child");
        exit(-4);
    } else if (id > 0) {
        close(fd[0]);

        char* str = read_string(STDIN);
        size_t str_size = str_length(str);
        if (write(fd[1], &str_size, sizeof(size_t)) != sizeof(size_t)) {
            perror("Write error");
            exit(3);
        }
        if (write(fd[1], str, sizeof(char) * str_size) != str_size) {
            perror("write error");
            exit(4);
        }

        if (close(fd[1]) == -1) {
            perror("Close error");
            exit(6);
        }

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
        
        free(str);
        printf("Parent exit\n");
    }
    else {
        close(fd[1]);

        size_t str_size;
        if (read(fd[0], &str_size, sizeof(size_t)) != sizeof(size_t)) {
            perror("Read error");
            exit(7);
        }
        char* str = (char*) malloc(sizeof(char) * str_size);
        if (str == NULL) {
            perror("Malloc error");
            exit(8);
        }
        if (read(fd[0], str, sizeof(char) * str_size) != str_size) {
            perror("Read error");
            exit(9);
        }
        
    
        if (close(fd[0]) == -1) {
            perror("Close error");
            exit(10);
        } 
    
        child_work(str, path);

        free(path);
        free(str);
        printf("Child exit\n");
    }

    return 0;
}