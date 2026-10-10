//drmemory -suppress mingw.supp -- .\task8.exe tests\in8.txt tests\o8.txt
// drmemory -suppress mingw.supp -- .\task8.exe tests\bad8.txt tests\o8b.txt

// .\task8.exe tests\in8.txt tests\o8.txt


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>

typedef enum {
    STATUS_OK,
    STATUS_END_OF_FILE,
    STATUS_NULL_POINTER,
    STATUS_INVALID_NUMBER,
    STATUS_OVERFLOW,
    STATUS_MEMORY_ERROR,
    STATUS_READ_ERROR,
    STATUS_WRITE_ERROR
} Status;

typedef struct {
    int is_negative;
    const char *digits;
    int base;
    unsigned long long value;
} Number;

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

Status char_to_digit(const char c, int *digit) {
    if (digit == NULL) {
        return STATUS_NULL_POINTER;
    }
    if (c >= '0' && c <= '9') {
        *digit = c - '0';
    } else if (c >= 'A' && c <= 'Z') {
        *digit = c - 'A' + 10;
    } else if (c >= 'a' && c <= 'z') {
        *digit = c - 'a' + 10;
    } else {
        return STATUS_INVALID_NUMBER;
    }
    return STATUS_OK;
}

Status analyze_number(const char *token, Number *number) {
    const char *p;
    int digit;
    int max_digit = 0;
    if (token == NULL || number == NULL) {
        return STATUS_NULL_POINTER;
    }
    number->is_negative = 0;
    if (*token == '-' || *token == '+') {
        number->is_negative = *token == '-';
        token++;
    }
    if (*token == '\0') {
        return STATUS_INVALID_NUMBER;
    }
    for (p = token; *p != '\0'; p++) {
        if (char_to_digit(*p, &digit) != STATUS_OK) {
            return STATUS_INVALID_NUMBER;
        }
        if (digit > max_digit) {
            max_digit = digit;
        }
    }
    while (*token == '0' && *(token + 1) != '\0') {
        token++;
    }
    number->digits = token;
    number->base = max_digit < 1 ? 2 : max_digit + 1;
    number->value = 0;
    for (p = token; *p != '\0'; p++) {
        char_to_digit(*p, &digit);
        if (number->value > (ULLONG_MAX - (unsigned long long)digit) / (unsigned long long)number->base) {
            return STATUS_OVERFLOW;
        }
        number->value = number->value * (unsigned long long)number->base + (unsigned long long)digit;
    }
    if (number->value == 0) {
        number->is_negative = 0;
    }
    return STATUS_OK;
}

Status write_number(FILE *out, const Number *number) {
    const char *sign;
    if (out == NULL || number == NULL) {
        return STATUS_NULL_POINTER;
    }
    sign = number->is_negative ? "-" : "";
    if (fprintf(out, "%s%s %d %s%llu\n", sign, number->digits, number->base, sign, number->value) < 0) {
        return STATUS_WRITE_ERROR;
    }
    return STATUS_OK;
}

Status process_file(FILE *in, FILE *out, char **bad_token) {
    char *token = NULL;
    size_t capacity = 0;
    Number number;
    Status status;
    if (in == NULL || out == NULL || bad_token == NULL) {
        return STATUS_NULL_POINTER;
    }
    *bad_token = NULL;
    while ((status = read_token(in, &token, &capacity)) == STATUS_OK) {
        status = analyze_number(token, &number);
        if (status != STATUS_OK) {
            *bad_token = token;
            return status;
        }
        status = write_number(out, &number);
        if (status != STATUS_OK) {
            break;
        }
    }
    free(token);
    return status == STATUS_END_OF_FILE ? STATUS_OK : status;
}

int main(int argc, char *argv[]) {
    FILE *in;
    FILE *out;
    char *bad_token;
    Status status;

    if (argc != 3) {
        printf("Использование: %s <входной файл> <выходной файл>\n", argv[0]);
        return 1;
    }
    if (strcmp(argv[1], argv[2]) == 0) {
        printf("Ошибка: входной и выходной файлы совпадают\n");
        return 1;
    }
    in = fopen(argv[1], "r");
    if (in == NULL) {
        printf("Ошибка: не удалось открыть файл %s\n", argv[1]);
        return 1;
    }
    out = fopen(argv[2], "w");
    if (out == NULL) {
        printf("Ошибка: не удалось открыть файл %s\n", argv[2]);
        fclose(in);
        return 1;
    }

    status = process_file(in, out, &bad_token);
    fclose(in);
    if (fclose(out) != 0 && status == STATUS_OK) {
        status = STATUS_WRITE_ERROR;
    }

    switch (status) {
        case STATUS_OK:
            printf("Результат записан в файл %s\n", argv[2]);
            break;
        case STATUS_INVALID_NUMBER:
            printf("Ошибка: \"%s\" не является числом ни в одной системе счисления [2..36]\n", bad_token);
            break;
        case STATUS_OVERFLOW:
            printf("Ошибка: число \"%s\" слишком большое\n", bad_token);
            break;
        case STATUS_MEMORY_ERROR:
            printf("Ошибка: не удалось выделить память\n");
            break;
        case STATUS_READ_ERROR:
            printf("Ошибка чтения входного файла\n");
            break;
        case STATUS_WRITE_ERROR:
            printf("Ошибка записи в выходной файл\n");
            break;
        default:
            printf("Ошибка обработки файла\n");
            break;
    }
    free(bad_token);
    return status == STATUS_OK ? 0 : 1;
}
