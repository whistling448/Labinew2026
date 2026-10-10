// .\task5.exe 0.000001 0.5

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <ctype.h>

typedef enum {
    STATUS_OK,
    STATUS_NULL_POINTER,
    STATUS_INVALID_NUMBER,
    STATUS_INVALID_EPSILON,
    STATUS_INVALID_X,
    STATUS_OVERFLOW,
    STATUS_ITERATION_LIMIT
} Status;

typedef Status (*Series)(double x, double eps, double *result);
typedef double (*Function)(double x);

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

Status check_eps(const double eps) {
    if (!isfinite(eps) || eps <= 0.0 || eps >= 1.0) {
        return STATUS_INVALID_EPSILON;
    }
    return STATUS_OK;
}

Status sum_a(double x, double eps, double *result) {
    double term = 1.0;
    double sum = 1.0;
    long n;
    const long max_terms = 100000000L;
    if (result == NULL) {
        return STATUS_NULL_POINTER;
    }
    if (check_eps(eps) != STATUS_OK) {
        return STATUS_INVALID_EPSILON;
    }
    for (n = 1; n < max_terms; n++) {
        term *= x / (double)n;
        sum += term;
        if (!isfinite(sum)) {
            return STATUS_OVERFLOW;
        }
        if (fabs(term) < eps && (double)n > fabs(x)) {
            *result = sum;
            return STATUS_OK;
        }
    }
    *result = sum;
    return STATUS_ITERATION_LIMIT;
}

Status sum_b(double x, double eps, double *result) {
    double term = 1.0;
    double sum = 1.0;
    long n;
    const long max_terms = 100000000L;
    if (result == NULL) {
        return STATUS_NULL_POINTER;
    }
    if (check_eps(eps) != STATUS_OK) {
        return STATUS_INVALID_EPSILON;
    }
    for (n = 0; n < max_terms; n++) {
        term *= -x * x / ((2.0 * n + 1.0) * (2.0 * n + 2.0));
        sum += term;
        if (!isfinite(sum)) {
            return STATUS_OVERFLOW;
        }
        if (fabs(term) < eps && 2.0 * n + 2.0 > fabs(x)) {
            *result = sum;
            return STATUS_OK;
        }
    }
    *result = sum;
    return STATUS_ITERATION_LIMIT;
}

Status sum_c(double x, double eps, double *result) {
    double term = 1.0;
    double sum = 1.0;
    long n;
    const long max_terms = 100000000L;
    if (result == NULL) {
        return STATUS_NULL_POINTER;
    }
    if (check_eps(eps) != STATUS_OK) {
        return STATUS_INVALID_EPSILON;
    }
    if (fabs(x) > 1.0 - eps) {
        return STATUS_INVALID_X;
    }
    for (n = 0; n < max_terms; n++) {
        term *= 9.0 * (n + 1.0) * (n + 1.0) * x * x / ((3.0 * n + 1.0) * (3.0 * n + 2.0));
        sum += term;
        if (fabs(term) < eps) {
            *result = sum;
            return STATUS_OK;
        }
    }
    *result = sum;
    return STATUS_ITERATION_LIMIT;
}

Status sum_d(double x, double eps, double *result) {
    double term;
    double sum;
    long n;
    const long max_terms = 100000000L;
    if (result == NULL) {
        return STATUS_NULL_POINTER;
    }
    if (check_eps(eps) != STATUS_OK) {
        return STATUS_INVALID_EPSILON;
    }
    if (fabs(x) > 1.0 - eps) {
        return STATUS_INVALID_X;
    }
    term = -x * x / 2.0;
    sum = term;
    for (n = 1; n < max_terms; n++) {
        if (fabs(term) < eps) {
            *result = sum;
            return STATUS_OK;
        }
        term *= -(2.0 * n + 1.0) * x * x / (2.0 * n + 2.0);
        sum += term;
    }
    *result = sum;
    return STATUS_ITERATION_LIMIT;
}

double function_a(double x) {
    return log(1.0 + x) / x;
}

double function_b(double x) {
    return exp(-x * x / 2.0);
}

double function_c(double x) {
    return -log(1.0 - x);
}

double function_d(double x) {
    return pow(x, x);
}

Status midpoint_rule(Function f, const double a, const double b, const long n, double *result) {
    double h;
    double sum = 0.0;
    long i;
    if (f == NULL || result == NULL) {
        return STATUS_NULL_POINTER;
    }
    h = (b - a) / (double)n;
    for (i = 0; i < n; i++) {
        sum += f(a + h * (i + 0.5));
    }
    *result = sum * h;
    return STATUS_OK;
}

Status integrate(Function f, const double a, const double b, const double eps, double *result) {
    double prev;
    double cur;
    const long max_splits = 1L << 24;
    long n;
    Status status;
    if (f == NULL || result == NULL) {
        return STATUS_NULL_POINTER;
    }
    if (check_eps(eps) != STATUS_OK) {
        return STATUS_INVALID_EPSILON;
    }
    status = midpoint_rule(f, a, b, 1, &prev);
    if (status != STATUS_OK) {
        return status;
    }
    for (n = 2; n <= max_splits; n *= 2) {
        status = midpoint_rule(f, a, b, n, &cur);
        if (status != STATUS_OK) {
            return status;
        }
        if (fabs(cur - prev) < eps) {
            *result = cur;
            return STATUS_OK;
        }
        prev = cur;
    }
    *result = prev;
    return STATUS_ITERATION_LIMIT;
}

void print_result(const char *name, const Status status, const double value) {
    switch (status) {
        case STATUS_OK:
            printf("  %s = %.12f\n", name, value);
            break;
        case STATUS_ITERATION_LIMIT:
            printf("  %s = %.12f (достигнут предел итераций)\n", name, value);
            break;
        case STATUS_INVALID_X:
            printf("  %s: ряд сходится только при |x| < 1\n", name);
            break;
        case STATUS_OVERFLOW:
            printf("  %s: переполнение\n", name);
            break;
        default:
            printf("  %s: ошибка вычисления\n", name);
            break;
    }
}

int main(int argc, char *argv[]) {
    const char *names[4] = {"a", "b", "c", "d"};
    Series sums[4] = {sum_a, sum_b, sum_c, sum_d};
    Function functions[4] = {function_a, function_b, function_c, function_d};
    double eps;
    double x;
    double value;
    Status status;
    int i;

    if (argc != 3) {
        printf("Использование: %s <эпсилон> <x>\n", argv[0]);
        return 1;
    }
    if (parse_double(argv[1], &eps) != STATUS_OK || check_eps(eps) != STATUS_OK) {
        printf("Ошибка: эпсилон должен быть вещественным числом в интервале (0, 1)\n");
        return 1;
    }
    if (parse_double(argv[2], &x) != STATUS_OK) {
        printf("Ошибка: x должен быть вещественным числом\n");
        return 1;
    }

    printf("Суммы рядов при x = %g:\n", x);
    for (i = 0; i < 4; i++) {
        value = 0.0;
        status = sums[i](x, eps, &value);
        print_result(names[i], status, value);
    }

    printf("Интегралы от 0 до 1:\n");
    for (i = 0; i < 4; i++) {
        value = 0.0;
        status = integrate(functions[i], 0.0, 1.0, eps, &value);
        print_result(names[i], status, value);
    }
    return 0;
}
