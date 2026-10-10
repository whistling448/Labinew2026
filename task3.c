// .\task3.exe -q 0.0001 1 -3 2
// .\task3.exe -m 10 5
// .\task3.exe -t 0.0001 3 4 5

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <ctype.h>
#include <limits.h>

typedef enum {
    STATUS_OK,
    STATUS_NULL_POINTER,
    STATUS_INVALID_NUMBER,
    STATUS_INVALID_EPSILON,
    STATUS_INVALID_VALUE,
    STATUS_OVERFLOW
} Status;

typedef enum {
    ROOTS_NONE,
    ROOTS_ONE,
    ROOTS_TWO,
    ROOTS_INFINITE
} RootsType;

typedef struct {
    double a;
    double b;
    double c;
    RootsType type;
    double x1;
    double x2;
} Equation;

Status parse_double(const char *str, double *result) {
    char *end;
    double value;
    if (str == NULL || result == NULL) {
        return STATUS_NULL_POINTER;
    }
    if (*str == '\0' || isspace((unsigned char)*str)) {
        return STATUS_INVALID_NUMBER;
    }
    value = strtod(str, &end);
    if (*end != '\0' || !isfinite(value)) {
        return STATUS_INVALID_NUMBER;
    }
    *result = value;
    return STATUS_OK;
}

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

Status parse_numbers(char *const strs[], const int count, double numbers[]) {
    int i;
    if (strs == NULL || numbers == NULL) {
        return STATUS_NULL_POINTER;
    }
    for (i = 0; i < count; i++) {
        if (parse_double(strs[i], &numbers[i]) != STATUS_OK) {
            return STATUS_INVALID_NUMBER;
        }
    }
    return STATUS_OK;
}

Status solve_quadratic(const double a, const double b, const double c, const double eps,
                       RootsType *type, double *x1, double *x2) {
    double d;
    if (type == NULL || x1 == NULL || x2 == NULL) {
        return STATUS_NULL_POINTER;
    }
    if (eps <= 0.0) {
        return STATUS_INVALID_EPSILON;
    }
    if (fabs(a) < eps) {
        if (fabs(b) < eps) {
            *type = fabs(c) < eps ? ROOTS_INFINITE : ROOTS_NONE;
        } else {
            *type = ROOTS_ONE;
            *x1 = -c / b + 0.0;
        }
        return STATUS_OK;
    }
    d = b * b - 4.0 * a * c;
    if (!isfinite(d)) {
        return STATUS_OVERFLOW;
    }
    if (fabs(d) < eps) {
        *type = ROOTS_ONE;
        *x1 = -b / (2.0 * a) + 0.0;
    } else if (d < 0.0) {
        *type = ROOTS_NONE;
    } else {
        *type = ROOTS_TWO;
        *x1 = (-b + sqrt(d)) / (2.0 * a) + 0.0;
        *x2 = (-b - sqrt(d)) / (2.0 * a) + 0.0;
    }
    return STATUS_OK;
}

int is_same_coefficients(const double first[3], const double second[3], const double eps) {
    return fabs(first[0] - second[0]) < eps && fabs(first[1] - second[1]) < eps &&
           fabs(first[2] - second[2]) < eps;
}

Status solve_permutations(const double coefficients[3], const double eps, Equation equations[], int *count) {
    const int orders[6][3] = {
        {0, 1, 2}, {0, 2, 1}, {1, 0, 2}, {1, 2, 0}, {2, 0, 1}, {2, 1, 0}
    };
    double current[3];
    double previous[3];
    int i;
    int j;
    int is_unique;
    Status status;
    if (coefficients == NULL || equations == NULL || count == NULL) {
        return STATUS_NULL_POINTER;
    }
    if (eps <= 0.0) {
        return STATUS_INVALID_EPSILON;
    }
    *count = 0;
    for (i = 0; i < 6; i++) {
        current[0] = coefficients[orders[i][0]];
        current[1] = coefficients[orders[i][1]];
        current[2] = coefficients[orders[i][2]];
        is_unique = 1;
        for (j = 0; j < *count; j++) {
            previous[0] = equations[j].a;
            previous[1] = equations[j].b;
            previous[2] = equations[j].c;
            if (is_same_coefficients(current, previous, eps)) {
                is_unique = 0;
                break;
            }
        }
        if (!is_unique) {
            continue;
        }
        equations[*count].a = current[0];
        equations[*count].b = current[1];
        equations[*count].c = current[2];
        status = solve_quadratic(current[0], current[1], current[2], eps,
                                 &equations[*count].type, &equations[*count].x1, &equations[*count].x2);
        if (status != STATUS_OK) {
            return status;
        }
        (*count)++;
    }
    return STATUS_OK;
}

Status is_multiple(const long a, const long b, int *result) {
    if (result == NULL) {
        return STATUS_NULL_POINTER;
    }
    if (a == 0 || b == 0) {
        return STATUS_INVALID_VALUE;
    }
    if (b == -1) {
        *result = 1;
    } else {
        *result = a % b == 0;
    }
    return STATUS_OK;
}

Status is_right_triangle(const double sides[3], const double eps, int *result) {
    double a;
    double b;
    double c;
    double tmp;
    if (sides == NULL || result == NULL) {
        return STATUS_NULL_POINTER;
    }
    if (eps <= 0.0) {
        return STATUS_INVALID_EPSILON;
    }
    a = sides[0];
    b = sides[1];
    c = sides[2];
    if (a < eps || b < eps || c < eps) {
        *result = 0;
        return STATUS_OK;
    }
    if (a > c) {
        tmp = a;
        a = c;
        c = tmp;
    }
    if (b > c) {
        tmp = b;
        b = c;
        c = tmp;
    }
    if (!isfinite(c * c)) {
        return STATUS_OVERFLOW;
    }
    *result = fabs(a * a + b * b - c * c) < eps;
    return STATUS_OK;
}

void print_equation(const Equation *equation) {
    printf("a = %g, b = %g, c = %g: ", equation->a, equation->b, equation->c);
    switch (equation->type) {
        case ROOTS_NONE:
            printf("нет действительных корней\n");
            break;
        case ROOTS_ONE:
            printf("x = %g\n", equation->x1);
            break;
        case ROOTS_TWO:
            printf("x1 = %g, x2 = %g\n", equation->x1, equation->x2);
            break;
        case ROOTS_INFINITE:
            printf("любое x является решением\n");
            break;
    }
}

int main(int argc, char *argv[]) {
    const char *flag;
    double numbers[4];
    Status status;

    if (argc < 2) {
        printf("Использование:\n");
        printf("  %s -q <эпсилон> <a> <b> <c>\n", argv[0]);
        printf("  %s -m <a> <b>\n", argv[0]);
        printf("  %s -t <эпсилон> <a> <b> <c>\n", argv[0]);
        return 1;
    }

    flag = argv[1];
    if ((flag[0] != '-' && flag[0] != '/') || flag[1] == '\0' || flag[2] != '\0') {
        printf("Ошибка: некорректный флаг %s\n", flag);
        return 1;
    }

    switch (flag[1]) {
        case 'q': {
            Equation equations[6];
            int count;
            int i;
            if (argc != 6) {
                printf("Ошибка: для флага -q нужно 4 числа: эпсилон и три коэффициента\n");
                return 1;
            }
            if (parse_numbers(argv + 2, 4, numbers) != STATUS_OK) {
                printf("Ошибка: параметры должны быть вещественными числами\n");
                return 1;
            }
            if (numbers[0] <= 0.0) {
                printf("Ошибка: эпсилон должен быть положительным\n");
                return 1;
            }
            status = solve_permutations(numbers + 1, numbers[0], equations, &count);
            if (status == STATUS_OVERFLOW) {
                printf("Ошибка: переполнение при вычислении дискриминанта\n");
                return 1;
            } else if (status != STATUS_OK) {
                printf("Ошибка при решении уравнений\n");
                return 1;
            }
            for (i = 0; i < count; i++) {
                print_equation(&equations[i]);
            }
            break;
        }
        case 'm': {
            long a;
            long b;
            int result;
            if (argc != 4) {
                printf("Ошибка: для флага -m нужно 2 целых числа\n");
                return 1;
            }
            if (parse_long(argv[2], &a) != STATUS_OK || parse_long(argv[3], &b) != STATUS_OK) {
                printf("Ошибка: параметры должны быть целыми числами\n");
                return 1;
            }
            status = is_multiple(a, b, &result);
            if (status == STATUS_INVALID_VALUE) {
                printf("Ошибка: числа должны быть ненулевыми\n");
                return 1;
            } else if (status != STATUS_OK) {
                printf("Ошибка при проверке кратности\n");
                return 1;
            }
            if (result) {
                printf("%ld кратно %ld\n", a, b);
            } else {
                printf("%ld не кратно %ld\n", a, b);
            }
            break;
        }
        case 't': {
            int result;
            if (argc != 6) {
                printf("Ошибка: для флага -t нужно 4 числа: эпсилон и три стороны\n");
                return 1;
            }
            if (parse_numbers(argv + 2, 4, numbers) != STATUS_OK) {
                printf("Ошибка: параметры должны быть вещественными числами\n");
                return 1;
            }
            if (numbers[0] <= 0.0) {
                printf("Ошибка: эпсилон должен быть положительным\n");
                return 1;
            }
            status = is_right_triangle(numbers + 1, numbers[0], &result);
            if (status == STATUS_OVERFLOW) {
                printf("Ошибка: переполнение при вычислениях\n");
                return 1;
            } else if (status != STATUS_OK) {
                printf("Ошибка при проверке треугольника\n");
                return 1;
            }
            if (result) {
                printf("Числа могут быть сторонами прямоугольного треугольника\n");
            } else {
                printf("Числа не могут быть сторонами прямоугольного треугольника\n");
            }
            break;
        }
        default:
            printf("Ошибка: неизвестный флаг %s\n", flag);
            return 1;
    }
    return 0;
}
