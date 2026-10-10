// drmemory -suppress mingw.supp -- .\task6.exe
// .\task6.exe

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <math.h>
#include <limits.h>

typedef enum {
    STATUS_OK,
    STATUS_NULL_POINTER,
    STATUS_INVALID_EPSILON,
    STATUS_INVALID_COUNT,
    STATUS_INVALID_BASE,
    STATUS_INVALID_NUMBER,
    STATUS_INVALID_INTERVAL,
    STATUS_DIVISION_BY_ZERO,
    STATUS_OVERFLOW,
    STATUS_MEMORY_ERROR,
    STATUS_ITERATION_LIMIT
} Status;

Status is_convex(int *result, const double eps, const int count, ...) {
    const double full_turn = 2.0 * acos(-1.0);
    double *points;
    double total_angle = 0.0;
    double ex1, ey1, ex2, ey2;
    double cross, dot;
    int sign = 0;
    int current;
    int i, j, k;
    va_list args;
    if (result == NULL) {
        return STATUS_NULL_POINTER;
    }
    if (eps <= 0.0) {
        return STATUS_INVALID_EPSILON;
    }
    if (count < 3 || count > INT_MAX / 2) {
        return STATUS_INVALID_COUNT;
    }
    points = (double *)malloc(2 * (size_t)count * sizeof(double));
    if (points == NULL) {
        return STATUS_MEMORY_ERROR;
    }
    va_start(args, count);
    for (i = 0; i < 2 * count; i++) {
        points[i] = va_arg(args, double);
    }
    va_end(args);

    *result = 1;
    for (i = 0; i < count; i++) {
        j = (i + 1) % count;
        k = (i + 2) % count;
        ex1 = points[2 * j] - points[2 * i];
        ey1 = points[2 * j + 1] - points[2 * i + 1];
        ex2 = points[2 * k] - points[2 * j];
        ey2 = points[2 * k + 1] - points[2 * j + 1];
        cross = ex1 * ey2 - ey1 * ex2;
        dot = ex1 * ex2 + ey1 * ey2;
        if (fabs(cross) < eps) {
            if (dot < -eps) {
                *result = 0;
                break;
            }
            continue;
        }
        current = cross > 0.0 ? 1 : -1;
        if (sign == 0) {
            sign = current;
        } else if (sign != current) {
            *result = 0;
            break;
        }
        total_angle += atan2(cross, dot);
    }
    if (*result && (sign == 0 || fabs(fabs(total_angle) - full_turn) > eps)) {
        *result = 0;
    }
    free(points);
    return STATUS_OK;
}

Status polynomial(double *result, const double x, const int degree, ...) {
    double value = 0.0;
    int i;
    va_list args;
    if (result == NULL) {
        return STATUS_NULL_POINTER;
    }
    if (degree < 0) {
        return STATUS_INVALID_COUNT;
    }
    va_start(args, degree);
    for (i = 0; i <= degree; i++) {
        value = value * x + va_arg(args, double);
    }
    va_end(args);
    if (!isfinite(value)) {
        return STATUS_OVERFLOW;
    }
    *result = value;
    return STATUS_OK;
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

Status parse_in_base(const char *str, const int base, unsigned long long *value) {
    int digit;
    if (str == NULL || value == NULL) {
        return STATUS_NULL_POINTER;
    }
    if (base < 2 || base > 36) {
        return STATUS_INVALID_BASE;
    }
    if (*str == '\0') {
        return STATUS_INVALID_NUMBER;
    }
    *value = 0;
    for (; *str != '\0'; str++) {
        if (char_to_digit(*str, &digit) != STATUS_OK || digit >= base) {
            return STATUS_INVALID_NUMBER;
        }
        if (*value > (ULLONG_MAX - (unsigned long long)digit) / (unsigned long long)base) {
            return STATUS_OVERFLOW;
        }
        *value = *value * (unsigned long long)base + (unsigned long long)digit;
    }
    return STATUS_OK;
}

Status is_kaprekar(const unsigned long long n, const int base, int *result) {
    unsigned long long square;
    unsigned long long power;
    unsigned long long left;
    unsigned long long right;
    const unsigned long long max_value = 4294967295ULL;
    if (result == NULL) {
        return STATUS_NULL_POINTER;
    }
    if (base < 2 || base > 36) {
        return STATUS_INVALID_BASE;
    }
    if (n > max_value) {
        return STATUS_OVERFLOW;
    }
    *result = 0;
    square = n * n;
    power = (unsigned long long)base;
    while (1) {
        left = square / power;
        right = square % power;
        if (right > 0 && left + right == n) {
            *result = 1;
            return STATUS_OK;
        }
        if (left == 0 || power > ULLONG_MAX / (unsigned long long)base) {
            return STATUS_OK;
        }
        power *= (unsigned long long)base;
    }
}

Status find_kaprekar(int flags[], const int base, const int count, ...) {
    const char *str;
    unsigned long long value;
    Status status = STATUS_OK;
    int i;
    va_list args;
    if (flags == NULL) {
        return STATUS_NULL_POINTER;
    }
    if (base < 2 || base > 36) {
        return STATUS_INVALID_BASE;
    }
    if (count < 0) {
        return STATUS_INVALID_COUNT;
    }
    va_start(args, count);
    for (i = 0; i < count && status == STATUS_OK; i++) {
        str = va_arg(args, const char *);
        status = parse_in_base(str, base, &value);
        if (status == STATUS_OK) {
            status = is_kaprekar(value, base, &flags[i]);
        }
    }
    va_end(args);
    return status;
}

Status geometric_mean(double *result, const double eps, const int count, ...) {
    double log_sum = 0.0;
    double value;
    int has_zero = 0;
    int i;
    va_list args;
    if (result == NULL) {
        return STATUS_NULL_POINTER;
    }
    if (eps <= 0.0) {
        return STATUS_INVALID_EPSILON;
    }
    if (count <= 0) {
        return STATUS_INVALID_COUNT;
    }
    va_start(args, count);
    for (i = 0; i < count; i++) {
        value = va_arg(args, double);
        if (value < -eps || !isfinite(value)) {
            va_end(args);
            return STATUS_INVALID_NUMBER;
        }
        if (fabs(value) < eps) {
            has_zero = 1;
        } else {
            log_sum += log(value);
        }
    }
    va_end(args);
    *result = has_zero ? 0.0 : exp(log_sum / count);
    return STATUS_OK;
}

Status power_recursive(const double base, const unsigned int n, double *result) {
    double half;
    Status status;
    if (result == NULL) {
        return STATUS_NULL_POINTER;
    }
    if (n == 0) {
        *result = 1.0;
        return STATUS_OK;
    }
    status = power_recursive(base, n / 2, &half);
    if (status != STATUS_OK) {
        return status;
    }
    *result = half * half;
    if (n % 2 == 1) {
        *result *= base;
    }
    if (!isfinite(*result)) {
        return STATUS_OVERFLOW;
    }
    return STATUS_OK;
}

Status fast_pow(double *result, const double base, const int exponent, const double eps) {
    unsigned int n;
    double value;
    Status status;
    if (result == NULL) {
        return STATUS_NULL_POINTER;
    }
    if (eps <= 0.0) {
        return STATUS_INVALID_EPSILON;
    }
    if (exponent < 0 && fabs(base) < eps) {
        return STATUS_DIVISION_BY_ZERO;
    }
    n = exponent < 0 ? 0U - (unsigned int)exponent : (unsigned int)exponent;
    status = power_recursive(base, n, &value);
    if (exponent < 0) {
        if (status == STATUS_OVERFLOW) {
            *result = 0.0;
            return STATUS_OK;
        }
        if (status != STATUS_OK) {
            return status;
        }
        value = 1.0 / value;
        if (!isfinite(value)) {
            return STATUS_OVERFLOW;
        }
    } else if (status != STATUS_OK) {
        return status;
    }
    *result = value;
    return STATUS_OK;
}

Status bisection(double *root, double (*f)(double), double a, double b, const double eps) {
    double fa;
    double fb;
    double mid;
    double fmid;
    const int max_steps = 1000;
    int i;
    if (root == NULL || f == NULL) {
        return STATUS_NULL_POINTER;
    }
    if (eps <= 0.0) {
        return STATUS_INVALID_EPSILON;
    }
    if (a > b - eps) {
        return STATUS_INVALID_INTERVAL;
    }
    fa = f(a);
    fb = f(b);
    if (!isfinite(fa) || !isfinite(fb) || fa * fb > 0.0) {
        return STATUS_INVALID_INTERVAL;
    }
    for (i = 0; i < max_steps && b - a > eps; i++) {
        mid = (a + b) / 2.0;
        fmid = f(mid);
        if (fa * fmid <= 0.0) {
            b = mid;
        } else {
            a = mid;
            fa = fmid;
        }
    }
    *root = (a + b) / 2.0;
    return b - a > eps ? STATUS_ITERATION_LIMIT : STATUS_OK;
}

double square_minus_two(double x) {
    return x * x - 2.0;
}

double cos_minus_x(double x) {
    return cos(x) - x;
}

double cubic(double x) {
    return x * x * x - x - 2.0;
}

double sine(double x) {
    return sin(x);
}

const char *status_message(const Status status) {
    switch (status) {
        case STATUS_OK:
            return "успешно";
        case STATUS_NULL_POINTER:
            return "нулевой указатель";
        case STATUS_INVALID_EPSILON:
            return "некорректный эпсилон";
        case STATUS_INVALID_COUNT:
            return "некорректное количество аргументов";
        case STATUS_INVALID_BASE:
            return "некорректное основание";
        case STATUS_INVALID_NUMBER:
            return "некорректное число";
        case STATUS_INVALID_INTERVAL:
            return "на интервале нет корня или интервал некорректен";
        case STATUS_DIVISION_BY_ZERO:
            return "деление на ноль";
        case STATUS_OVERFLOW:
            return "переполнение";
        case STATUS_MEMORY_ERROR:
            return "не удалось выделить память";
        case STATUS_ITERATION_LIMIT:
            return "достигнут предел итераций";
    }
    return "неизвестная ошибка";
}

void demo_convex(void) {
    const double eps = 1e-9;
    int result;
    Status status;

    printf("1. Выпуклость многоугольника\n");
    status = is_convex(&result, eps, 4, 0.0, 0.0, 1.0, 0.0, 1.0, 1.0, 0.0, 1.0);
    if (status == STATUS_OK) {
        printf("  квадрат: %s\n", result ? "выпуклый" : "невыпуклый");
    } else {
        printf("  квадрат: ошибка - %s\n", status_message(status));
    }
    status = is_convex(&result, eps, 4, 0.0, 0.0, 2.0, 0.0, 1.0, 0.5, 1.0, 2.0);
    if (status == STATUS_OK) {
        printf("  четырёхугольник с вогнутостью: %s\n", result ? "выпуклый" : "невыпуклый");
    } else {
        printf("  четырёхугольник с вогнутостью: ошибка - %s\n", status_message(status));
    }
    status = is_convex(&result, eps, 5, 0.0, 0.0, 2.0, 0.0, 3.0, 1.5, 1.0, 3.0, -1.0, 1.5);
    if (status == STATUS_OK) {
        printf("  пятиугольник: %s\n", result ? "выпуклый" : "невыпуклый");
    } else {
        printf("  пятиугольник: ошибка - %s\n", status_message(status));
    }
    status = is_convex(&result, eps, 2, 0.0, 0.0, 1.0, 1.0);
    if (status == STATUS_OK) {
        printf("  две точки: %s\n", result ? "выпуклый" : "невыпуклый");
    } else {
        printf("  две точки: ошибка - %s\n", status_message(status));
    }
}

void demo_polynomial(void) {
    double result;
    Status status;

    printf("2. Значение многочлена\n");
    status = polynomial(&result, 2.0, 2, 1.0, -3.0, 2.0);
    if (status == STATUS_OK) {
        printf("  x^2 - 3x + 2 при x = 2: %g\n", result);
    } else {
        printf("  ошибка - %s\n", status_message(status));
    }
    status = polynomial(&result, -1.5, 3, 2.0, 0.0, -1.0, 4.0);
    if (status == STATUS_OK) {
        printf("  2x^3 - x + 4 при x = -1.5: %g\n", result);
    } else {
        printf("  ошибка - %s\n", status_message(status));
    }
    status = polynomial(&result, 10.0, 0, 7.0);
    if (status == STATUS_OK) {
        printf("  7 при x = 10: %g\n", result);
    } else {
        printf("  ошибка - %s\n", status_message(status));
    }
}

void print_kaprekar(const int base, const int count, const char *const numbers[], const int flags[]) {
    int i;
    printf("  основание %d: ", base);
    for (i = 0; i < count; i++) {
        if (flags[i]) {
            printf("%s ", numbers[i]);
        }
    }
    printf("\n");
}

void demo_kaprekar(void) {
    const char *decimal[] = {"1", "9", "10", "45", "55", "100", "297", "703", "998", "999", "2223"};
    const char *hex[] = {"1", "6", "A", "B", "f", "33", "50", "FF", "15F"};
    const char *binary[] = {"1", "10", "11", "110", "111", "1000", "1010"};
    int decimal_flags[11];
    int hex_flags[9];
    int binary_flags[7];
    Status status;

    printf("3. Числа Капрекара\n");
    status = find_kaprekar(decimal_flags, 10, 11, decimal[0], decimal[1], decimal[2], decimal[3],
                           decimal[4], decimal[5], decimal[6], decimal[7], decimal[8], decimal[9], decimal[10]);
    if (status == STATUS_OK) {
        print_kaprekar(10, 11, decimal, decimal_flags);
    } else {
        printf("  ошибка - %s\n", status_message(status));
    }
    status = find_kaprekar(hex_flags, 16, 9, hex[0], hex[1], hex[2], hex[3], hex[4], hex[5], hex[6],
                           hex[7], hex[8]);
    if (status == STATUS_OK) {
        print_kaprekar(16, 9, hex, hex_flags);
    } else {
        printf("  ошибка - %s\n", status_message(status));
    }
    status = find_kaprekar(binary_flags, 2, 7, binary[0], binary[1], binary[2], binary[3], binary[4],
                           binary[5], binary[6]);
    if (status == STATUS_OK) {
        print_kaprekar(2, 7, binary, binary_flags);
    } else {
        printf("  ошибка - %s\n", status_message(status));
    }
    status = find_kaprekar(binary_flags, 2, 2, "101", "123");
    if (status != STATUS_OK) {
        printf("  основание 2, число 123: ошибка - %s\n", status_message(status));
    }
}

void demo_geometric_mean(void) {
    const double eps = 1e-9;
    double result;
    Status status;

    printf("4. Среднее геометрическое\n");
    status = geometric_mean(&result, eps, 3, 2.0, 8.0, 4.0);
    if (status == STATUS_OK) {
        printf("  2, 8, 4: %g\n", result);
    } else {
        printf("  ошибка - %s\n", status_message(status));
    }
    status = geometric_mean(&result, eps, 4, 1.0, 3.0, 9.0, 27.0);
    if (status == STATUS_OK) {
        printf("  1, 3, 9, 27: %g\n", result);
    } else {
        printf("  ошибка - %s\n", status_message(status));
    }
    status = geometric_mean(&result, eps, 2, 5.0, -5.0);
    if (status == STATUS_OK) {
        printf("  5, -5: %g\n", result);
    } else {
        printf("  5, -5: ошибка - %s\n", status_message(status));
    }
}

void demo_fast_pow(void) {
    const double eps = 1e-12;
    const double bases[] = {2.0, 2.0, -3.0, 1.5, 0.0, 10.0};
    const int exponents[] = {10, -3, 5, 0, -1, 400};
    double result;
    Status status;
    int i;

    printf("5. Быстрое возведение в степень\n");
    for (i = 0; i < 6; i++) {
        status = fast_pow(&result, bases[i], exponents[i], eps);
        if (status == STATUS_OK) {
            printf("  %g ^ %d = %g\n", bases[i], exponents[i], result);
        } else {
            printf("  %g ^ %d: ошибка - %s\n", bases[i], exponents[i], status_message(status));
        }
    }
}

void demo_bisection(void) {
    double (*functions[])(double) = {square_minus_two, square_minus_two, cos_minus_x, cubic, sine, square_minus_two};
    const char *names[] = {"x^2 - 2", "x^2 - 2", "cos(x) - x", "x^3 - x - 2", "sin(x)", "x^2 - 2"};
    const double left[] = {0.0, 0.0, 0.0, 1.0, 3.0, 2.0};
    const double right[] = {2.0, 2.0, 1.0, 2.0, 4.0, 3.0};
    const double epsilons[] = {1e-3, 1e-10, 1e-6, 1e-8, 1e-12, 1e-6};
    double root;
    Status status;
    int i;

    printf("6. Метод дихотомии\n");
    for (i = 0; i < 6; i++) {
        status = bisection(&root, functions[i], left[i], right[i], epsilons[i]);
        if (status == STATUS_OK) {
            printf("  %s на [%g, %g], eps = %g: x = %.12f\n", names[i], left[i], right[i], epsilons[i], root);
        } else {
            printf("  %s на [%g, %g], eps = %g: ошибка - %s\n", names[i], left[i], right[i], epsilons[i],
                   status_message(status));
        }
    }
}

int main(void) {
    demo_convex();
    demo_polynomial();
    demo_kaprekar();
    demo_geometric_mean();
    demo_fast_pow();
    demo_bisection();
    return 0;
}
