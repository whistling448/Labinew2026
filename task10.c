// drmemory -suppress mingw.supp -- .\task10.exe

// .\task10.exe
// 16
// 1A -FF 0010 7FFF
// Stop


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>

typedef enum {
    STATUS_OK,
    STATUS_END_OF_FILE,
    STATUS_NULL_POINTER,
    STATUS_INVALID_BASE,
    STATUS_INVALID_NUMBER,
    STATUS_OVERFLOW,
    STATUS_MEMORY_ERROR,
    STATUS_READ_ERROR
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

Status parse_base(const char *str, int *base) {
    int value = 0;
    if (str == NULL || base == NULL) {
        return STATUS_NULL_POINTER;
    }
    if (*str == '\0' || strlen(str) > 2) {
        return STATUS_INVALID_BASE;
    }
    for (; *str != '\0'; str++) {
        if (*str < '0' || *str > '9') {
            return STATUS_INVALID_BASE;
        }
        value = value * 10 + (*str - '0');
    }
    if (value < 2 || value > 36) {
        return STATUS_INVALID_BASE;
    }
    *base = value;
    return STATUS_OK;
}

Status parse_number(const char *str, const int base, long long *result) {
    unsigned long long magnitude = 0;
    unsigned long long limit = (unsigned long long)LLONG_MAX;
    int is_negative = 0;
    int digit;
    if (str == NULL || result == NULL) {
        return STATUS_NULL_POINTER;
    }
    if (base < 2 || base > 36) {
        return STATUS_INVALID_BASE;
    }
    if (*str == '-') {
        is_negative = 1;
        limit += 1ULL;
        str++;
    }
    if (*str == '\0') {
        return STATUS_INVALID_NUMBER;
    }
    for (; *str != '\0'; str++) {
        if (*str >= '0' && *str <= '9') {
            digit = *str - '0';
        } else if (*str >= 'A' && *str <= 'Z') {
            digit = *str - 'A' + 10;
        } else {
            return STATUS_INVALID_NUMBER;
        }
        if (digit >= base) {
            return STATUS_INVALID_NUMBER;
        }
        if (magnitude > (limit - (unsigned long long)digit) / (unsigned long long)base) {
            return STATUS_OVERFLOW;
        }
        magnitude = magnitude * (unsigned long long)base + (unsigned long long)digit;
    }
    if (is_negative) {
        *result = magnitude == 0 ? 0 : -(long long)(magnitude - 1ULL) - 1LL;
    } else {
        *result = (long long)magnitude;
    }
    return STATUS_OK;
}

unsigned long long absolute(const long long value) {
    return value < 0 ? 0ULL - (unsigned long long)value : (unsigned long long)value;
}

Status add_number(const long long value, long long *sum, long long *max_abs, int *count) {
    if (sum == NULL || max_abs == NULL || count == NULL) {
        return STATUS_NULL_POINTER;
    }
    if ((value > 0 && *sum > LLONG_MAX - value) || (value < 0 && *sum < LLONG_MIN - value)) {
        return STATUS_OVERFLOW;
    }
    *sum += value;
    if (*count == 0 || absolute(value) > absolute(*max_abs)) {
        *max_abs = value;
    }
    (*count)++;
    return STATUS_OK;
}

Status to_base(const long long value, const int base, char *out, const size_t size) {
    const char alphabet[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    char digits[70];
    unsigned long long magnitude = absolute(value);
    size_t count = 0;
    size_t position = 0;
    if (out == NULL) {
        return STATUS_NULL_POINTER;
    }
    if (base < 2 || base > 36) {
        return STATUS_INVALID_BASE;
    }
    do {
        digits[count] = alphabet[magnitude % (unsigned long long)base];
        count++;
        magnitude /= (unsigned long long)base;
    } while (magnitude > 0);
    if (count + 2 > size) {
        return STATUS_OVERFLOW;
    }
    if (value < 0) {
        out[position] = '-';
        position++;
    }
    while (count > 0) {
        count--;
        out[position] = digits[count];
        position++;
    }
    out[position] = '\0';
    return STATUS_OK;
}

void print_number(const char *name, const long long value, const int input_base) {
    const int bases[4] = {9, 18, 27, 36};
    char buffer[70];
    int i;
    if (to_base(value, input_base, buffer, sizeof(buffer)) != STATUS_OK) {
        printf("Ошибка перевода числа\n");
        return;
    }
    printf("%s: %s\n", name, buffer);
    for (i = 0; i < 4; i++) {
        if (to_base(value, bases[i], buffer, sizeof(buffer)) == STATUS_OK) {
            printf("  в системе с основанием %d: %s\n", bases[i], buffer);
        }
    }
}

int main(void) {
    char *token = NULL;
    size_t capacity = 0;
    int base;
    long long value;
    long long sum = 0;
    long long max_abs = 0;
    int count = 0;
    int stopped = 0;
    Status status;

    printf("Введите основание системы счисления [2..36]: ");
    status = read_token(stdin, &token, &capacity);
    if (status != STATUS_OK) {
        printf("\nОшибка: основание не введено\n");
        free(token);
        return 1;
    }
    if (parse_base(token, &base) != STATUS_OK) {
        printf("Ошибка: основание должно быть целым числом от 2 до 36\n");
        free(token);
        return 1;
    }

    printf("Введите числа (для завершения введите Stop):\n");
    while ((status = read_token(stdin, &token, &capacity)) == STATUS_OK) {
        if (strcmp(token, "Stop") == 0) {
            stopped = 1;
            break;
        }
        status = parse_number(token, base, &value);
        if (status == STATUS_INVALID_NUMBER) {
            printf("Ошибка: \"%s\" не является числом в системе с основанием %d, пропущено\n", token, base);
            continue;
        } else if (status == STATUS_OVERFLOW) {
            printf("Ошибка: число \"%s\" слишком большое, пропущено\n", token);
            continue;
        } else if (status != STATUS_OK) {
            break;
        }
        if (add_number(value, &sum, &max_abs, &count) != STATUS_OK) {
            printf("Ошибка: переполнение суммы, число \"%s\" пропущено\n", token);
        }
    }
    free(token);

    if (status == STATUS_MEMORY_ERROR) {
        printf("Ошибка: не удалось выделить память\n");
        return 1;
    }
    if (status == STATUS_READ_ERROR) {
        printf("Ошибка чтения ввода\n");
        return 1;
    }
    if (!stopped) {
        printf("Ввод завершён без строки Stop\n");
    }
    if (count == 0) {
        printf("Не введено ни одного числа\n");
        return 0;
    }

    print_number("Максимальное по модулю", max_abs, base);
    print_number("Сумма", sum, base);
    return 0;
}
