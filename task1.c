// .\task1.exe 15 -h

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <limits.h>

typedef enum {
    STATUS_OK,
    STATUS_NULL_POINTER,
    STATUS_INVALID_NUMBER,
    STATUS_OUT_OF_RANGE,
    STATUS_OVERFLOW
} Status;

Status parse_long(const char *str, long *result) {
    const char *p;
    long value = 0;
    int negative = 0;
    int digit;
    if (str == NULL || result == NULL) {
        return STATUS_NULL_POINTER;
    }
    p = str;
    if (*p == '-' || *p == '+') {
        negative = *p == '-';
        p++;
    }
    if (*p == '\0') {
        return STATUS_INVALID_NUMBER;
    }
    for (; *p != '\0'; p++) {
        if (!isdigit((unsigned char)*p)) {
            return STATUS_INVALID_NUMBER;
        }
        digit = *p - '0';
        if (negative) {
            if (value < (LONG_MIN + digit) / 10) {
                return STATUS_OVERFLOW;
            }
            value = value * 10 - digit;
        } else {
            if (value > (LONG_MAX - digit) / 10) {
                return STATUS_OVERFLOW;
            }
            value = value * 10 + digit;
        }
    }
    *result = value;
    return STATUS_OK;
}

Status find_multiples(const long x, const long limit, long multiples[], int *count) {
    long i;
    if (multiples == NULL || count == NULL) {
        return STATUS_NULL_POINTER;
    }
    if (x <= 0) {
        return STATUS_OUT_OF_RANGE;
    }
    *count = 0;
    for (i = x; i <= limit; i += x) {
        multiples[*count] = i;
        (*count)++;
    }
    return STATUS_OK;
}

Status check_prime(const long x, int *is_prime, int *is_composite) {
    long i;
    if (is_prime == NULL || is_composite == NULL) {
        return STATUS_NULL_POINTER;
    }
    if (x < 0) {
        return STATUS_OUT_OF_RANGE;
    }
    *is_prime = 0;
    *is_composite = 0;
    if (x < 2) {
        return STATUS_OK;
    }
    for (i = 2; i <= x / i; i++) {
        if (x % i == 0) {
            *is_composite = 1;
            return STATUS_OK;
        }
    }
    *is_prime = 1;
    return STATUS_OK;
}

Status split_hex(const long x, char digits[], int *count, int *is_negative) {
    const char alphabet[] = "0123456789ABCDEF";
    unsigned long value;
    int i;
    char tmp;
    if (digits == NULL || count == NULL || is_negative == NULL) {
        return STATUS_NULL_POINTER;
    }
    *is_negative = x < 0;
    value = x < 0 ? 0UL - (unsigned long)x : (unsigned long)x;
    *count = 0;
    do {
        digits[*count] = alphabet[value % 16];
        (*count)++;
        value /= 16;
    } while (value > 0);
    for (i = 0; i < *count / 2; i++) {
        tmp = digits[i];
        digits[i] = digits[*count - 1 - i];
        digits[*count - 1 - i] = tmp;
    }
    return STATUS_OK;
}

Status power_table(const long x, const int max_power, long long table[10][10]) {
    int base;
    int exponent;
    if (table == NULL) {
        return STATUS_NULL_POINTER;
    }
    if (x < 1 || x > max_power) {
        return STATUS_OUT_OF_RANGE;
    }
    for (base = 1; base <= max_power; base++) {
        table[base - 1][0] = base;
        for (exponent = 2; exponent <= x; exponent++) {
            table[base - 1][exponent - 1] = table[base - 1][exponent - 2] * base;
        }
    }
    return STATUS_OK;
}

Status sum_to(const long x, unsigned long long *sum) {
    unsigned long long a;
    unsigned long long b;
    if (sum == NULL) {
        return STATUS_NULL_POINTER;
    }
    if (x < 1) {
        return STATUS_OUT_OF_RANGE;
    }
    a = (unsigned long long)x;
    b = a + 1;
    if (a % 2 == 0) {
        a /= 2;
    } else {
        b /= 2;
    }
    if (a > ULLONG_MAX / b) {
        return STATUS_OVERFLOW;
    }
    *sum = a * b;
    return STATUS_OK;
}

Status factorial(const long x, unsigned long long *result) {
    unsigned long long value = 1;
    long i;
    if (result == NULL) {
        return STATUS_NULL_POINTER;
    }
    if (x < 0) {
        return STATUS_OUT_OF_RANGE;
    }
    for (i = 2; i <= x; i++) {
        if (value > ULLONG_MAX / (unsigned long long)i) {
            return STATUS_OVERFLOW;
        }
        value *= (unsigned long long)i;
    }
    *result = value;
    return STATUS_OK;
}

void print_status(const Status status) {
    switch (status) {
        case STATUS_NULL_POINTER:
            printf("Ошибка: передан нулевой указатель\n");
            break;
        case STATUS_INVALID_NUMBER:
            printf("Ошибка: x не является целым числом\n");
            break;
        case STATUS_OUT_OF_RANGE:
            printf("Ошибка: значение x недопустимо\n");
            break;
        case STATUS_OVERFLOW:
            printf("Ошибка: переполнение\n");
            break;
        default:
            break;
    }
}

int main(int argc, char *argv[]) {
    const long max_multiple = 100;
    const int max_power = 10;
    long x;
    Status status;
    const char *flag;

    if (argc != 3) {
        printf("Использование: %s <x> <флаг>\n", argv[0]);
        printf("Флаги: -h -p -s -e -a -f (или /h /p ...)\n");
        return 1;
    }

    status = parse_long(argv[1], &x);
    if (status != STATUS_OK) {
        print_status(status);
        return 1;
    }

    flag = argv[2];
    if ((flag[0] != '-' && flag[0] != '/') || flag[1] == '\0' || flag[2] != '\0') {
        printf("Ошибка: некорректный флаг %s\n", flag);
        return 1;
    }

    switch (flag[1]) {
        case 'h': {
            long multiples[100];
            int count;
            int i;
            status = find_multiples(x, max_multiple, multiples, &count);
            if (status != STATUS_OK) {
                print_status(status);
                return 1;
            }
            if (count == 0) {
                printf("Нет натуральных чисел до %ld, кратных %ld\n", max_multiple, x);
            } else {
                for (i = 0; i < count; i++) {
                    printf("%ld ", multiples[i]);
                }
                printf("\n");
            }
            break;
        }
        case 'p': {
            int is_prime;
            int is_composite;
            status = check_prime(x, &is_prime, &is_composite);
            if (status != STATUS_OK) {
                print_status(status);
                return 1;
            }
            if (is_prime) {
                printf("%ld - простое число\n", x);
            } else if (is_composite) {
                printf("%ld - составное число\n", x);
            } else {
                printf("%ld - не является ни простым, ни составным\n", x);
            }
            break;
        }
        case 's': {
            char digits[16];
            int count;
            int is_negative;
            int i;
            status = split_hex(x, digits, &count, &is_negative);
            if (status != STATUS_OK) {
                print_status(status);
                return 1;
            }
            if (is_negative) {
                printf("- ");
            }
            for (i = 0; i < count; i++) {
                printf("%c ", digits[i]);
            }
            printf("\n");
            break;
        }
        case 'e': {
            long long table[10][10];
            int base;
            int exponent;
            status = power_table(x, max_power, table);
            if (status != STATUS_OK) {
                print_status(status);
                printf("Для флага -e x должен быть от 1 до %d\n", max_power);
                return 1;
            }
            printf("%6s", "осн.");
            for (exponent = 1; exponent <= x; exponent++) {
                printf("%12d", exponent);
            }
            printf("\n");
            for (base = 1; base <= max_power; base++) {
                printf("%6d", base);
                for (exponent = 1; exponent <= x; exponent++) {
                    printf("%12lld", table[base - 1][exponent - 1]);
                }
                printf("\n");
            }
            break;
        }
        case 'a': {
            unsigned long long sum;
            status = sum_to(x, &sum);
            if (status != STATUS_OK) {
                print_status(status);
                return 1;
            }
            printf("Сумма чисел от 1 до %ld = %llu\n", x, sum);
            break;
        }
        case 'f': {
            unsigned long long result;
            status = factorial(x, &result);
            if (status != STATUS_OK) {
                print_status(status);
                return 1;
            }
            printf("%ld! = %llu\n", x, result);
            break;
        }
        default:
            printf("Ошибка: неизвестный флаг %s\n", flag);
            return 1;
    }

    return 0;
}
