// drmemory -suppress mingw.supp -- .\task7.exe -r tests\a7.txt tests\b7.txt tests\o7r.txt
// drmemory -suppress mingw.supp -- .\task7.exe -a tests\c7.txt tests\o7a.txt
// drmemory -suppress mingw.supp -- .\task7.exe -r tests\a7.txt нет_файла tests\o.txt

//.\task7.exe -r tests\a7.txt tests\b7.txt tests\o7r.txt
//.\task7.exe -a tests\c7.txt tests\o7a.txt

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef enum {
    STATUS_OK,
    STATUS_END_OF_FILE,
    STATUS_NULL_POINTER,
    STATUS_INVALID_BASE,
    STATUS_MEMORY_ERROR,
    STATUS_READ_ERROR,
    STATUS_WRITE_ERROR
} Status;

Status read_token(FILE *file, char **buffer, size_t *capacity) {
    const size_t start_capacity = 16;
    size_t length = 0;
    char *new_buffer;
    int c;
    if (file == NULL || buffer == NULL || capacity == NULL) {
        return STATUS_NULL_POINTER;
    }
    if (*buffer == NULL) {
        *buffer = (char *)malloc(start_capacity);
        if (*buffer == NULL) {
            return STATUS_MEMORY_ERROR;
        }
        *capacity = start_capacity;
    }
    do {
        c = fgetc(file);
    } while (c != EOF && isspace(c));
    while (c != EOF && !isspace(c)) {
        if (length + 1 >= *capacity) {
            new_buffer = (char *)realloc(*buffer, *capacity * 2);
            if (new_buffer == NULL) {
                return STATUS_MEMORY_ERROR;
            }
            *buffer = new_buffer;
            *capacity *= 2;
        }
        (*buffer)[length] = (char)c;
        length++;
        c = fgetc(file);
    }
    if (ferror(file)) {
        return STATUS_READ_ERROR;
    }
    (*buffer)[length] = '\0';
    return length == 0 ? STATUS_END_OF_FILE : STATUS_OK;
}

Status write_token(FILE *file, const char *token, int *is_first) {
    if (file == NULL || token == NULL || is_first == NULL) {
        return STATUS_NULL_POINTER;
    }
    if (!*is_first && fputc(' ', file) == EOF) {
        return STATUS_WRITE_ERROR;
    }
    if (fputs(token, file) == EOF) {
        return STATUS_WRITE_ERROR;
    }
    *is_first = 0;
    return STATUS_OK;
}

Status to_base(unsigned int value, const int base, char *out) {
    char digits[16];
    int count = 0;
    int i;
    if (out == NULL) {
        return STATUS_NULL_POINTER;
    }
    if (base < 2 || base > 10) {
        return STATUS_INVALID_BASE;
    }
    do {
        digits[count] = (char)('0' + value % (unsigned int)base);
        count++;
        value /= (unsigned int)base;
    } while (value > 0);
    for (i = 0; i < count; i++) {
        out[i] = digits[count - 1 - i];
    }
    out[count] = '\0';
    return STATUS_OK;
}

Status transform_token(const char *token, const long index, char **result) {
    size_t length;
    size_t i;
    char code[16];
    int base = 0;
    int to_lower = 0;
    if (token == NULL || result == NULL) {
        return STATUS_NULL_POINTER;
    }
    if (index % 10 == 0) {
        to_lower = 1;
        base = 4;
    } else if (index % 2 == 0) {
        to_lower = 1;
    } else if (index % 5 == 0) {
        base = 8;
    }
    length = strlen(token);
    *result = (char *)malloc(length * 4 + 1);
    if (*result == NULL) {
        return STATUS_MEMORY_ERROR;
    }
    (*result)[0] = '\0';
    for (i = 0; i < length; i++) {
        unsigned char c = (unsigned char)token[i];
        if (to_lower && c >= 'A' && c <= 'Z') {
            c = (unsigned char)(c - 'A' + 'a');
        }
        if (base != 0) {
            to_base(c, base, code);
            strcat(*result, code);
        } else {
            code[0] = (char)c;
            code[1] = '\0';
            strcat(*result, code);
        }
    }
    return STATUS_OK;
}

Status merge_files(FILE *first, FILE *second, FILE *out) {
    char *token1 = NULL;
    char *token2 = NULL;
    size_t capacity1 = 0;
    size_t capacity2 = 0;
    int is_first = 1;
    Status status1;
    Status status2;
    Status status = STATUS_OK;
    if (first == NULL || second == NULL || out == NULL) {
        return STATUS_NULL_POINTER;
    }
    status1 = read_token(first, &token1, &capacity1);
    status2 = read_token(second, &token2, &capacity2);
    while (status1 == STATUS_OK || status2 == STATUS_OK) {
        if (status1 == STATUS_OK) {
            status = write_token(out, token1, &is_first);
            if (status != STATUS_OK) {
                break;
            }
            status1 = read_token(first, &token1, &capacity1);
        }
        if (status2 == STATUS_OK) {
            status = write_token(out, token2, &is_first);
            if (status != STATUS_OK) {
                break;
            }
            status2 = read_token(second, &token2, &capacity2);
        }
    }
    if (status == STATUS_OK && status1 != STATUS_END_OF_FILE) {
        status = status1;
    }
    if (status == STATUS_OK && status2 != STATUS_END_OF_FILE) {
        status = status2;
    }
    free(token1);
    free(token2);
    return status;
}

Status convert_file(FILE *in, FILE *out) {
    char *token = NULL;
    char *converted;
    size_t capacity = 0;
    long index = 0;
    int is_first = 1;
    Status status;
    if (in == NULL || out == NULL) {
        return STATUS_NULL_POINTER;
    }
    while ((status = read_token(in, &token, &capacity)) == STATUS_OK) {
        index++;
        status = transform_token(token, index, &converted);
        if (status != STATUS_OK) {
            break;
        }
        status = write_token(out, converted, &is_first);
        free(converted);
        if (status != STATUS_OK) {
            break;
        }
    }
    free(token);
    return status == STATUS_END_OF_FILE ? STATUS_OK : status;
}

void print_status(const Status status) {
    switch (status) {
        case STATUS_OK:
            printf("Результат записан в выходной файл\n");
            break;
        case STATUS_MEMORY_ERROR:
            printf("Ошибка: не удалось выделить память\n");
            break;
        case STATUS_READ_ERROR:
            printf("Ошибка чтения файла\n");
            break;
        case STATUS_WRITE_ERROR:
            printf("Ошибка записи в файл\n");
            break;
        default:
            printf("Ошибка обработки файлов\n");
            break;
    }
}

int main(int argc, char *argv[]) {
    const char *flag;
    FILE *first;
    FILE *second;
    FILE *out;
    Status status;

    if (argc < 2) {
        printf("Использование:\n");
        printf("  %s -r <файл1> <файл2> <выходной файл>\n", argv[0]);
        printf("  %s -a <файл> <выходной файл>\n", argv[0]);
        return 1;
    }

    flag = argv[1];
    if ((flag[0] != '-' && flag[0] != '/') || flag[1] == '\0' || flag[2] != '\0') {
        printf("Ошибка: некорректный флаг %s\n", flag);
        return 1;
    }

    if (flag[1] == 'r') {
        if (argc != 5) {
            printf("Ошибка: для флага -r нужно 3 пути к файлам\n");
            return 1;
        }
        if (strcmp(argv[4], argv[2]) == 0 || strcmp(argv[4], argv[3]) == 0) {
            printf("Ошибка: выходной файл совпадает с входным\n");
            return 1;
        }
        first = fopen(argv[2], "r");
        if (first == NULL) {
            printf("Ошибка: не удалось открыть файл %s\n", argv[2]);
            return 1;
        }
        second = fopen(argv[3], "r");
        if (second == NULL) {
            printf("Ошибка: не удалось открыть файл %s\n", argv[3]);
            fclose(first);
            return 1;
        }
        out = fopen(argv[4], "w");
        if (out == NULL) {
            printf("Ошибка: не удалось открыть файл %s\n", argv[4]);
            fclose(first);
            fclose(second);
            return 1;
        }
        status = merge_files(first, second, out);
        fclose(first);
        fclose(second);
    } else if (flag[1] == 'a') {
        if (argc != 4) {
            printf("Ошибка: для флага -a нужно 2 пути к файлам\n");
            return 1;
        }
        if (strcmp(argv[2], argv[3]) == 0) {
            printf("Ошибка: выходной файл совпадает с входным\n");
            return 1;
        }
        first = fopen(argv[2], "r");
        if (first == NULL) {
            printf("Ошибка: не удалось открыть файл %s\n", argv[2]);
            return 1;
        }
        out = fopen(argv[3], "w");
        if (out == NULL) {
            printf("Ошибка: не удалось открыть файл %s\n", argv[3]);
            fclose(first);
            return 1;
        }
        status = convert_file(first, out);
        fclose(first);
    } else {
        printf("Ошибка: неизвестный флаг %s\n", flag);
        return 1;
    }

    if (fclose(out) != 0 && status == STATUS_OK) {
        status = STATUS_WRITE_ERROR;
    }
    print_status(status);
    return status == STATUS_OK ? 0 : 1;
}
