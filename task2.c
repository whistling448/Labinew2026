// .\task2.exe 0.0001
// drmemory -suppress mingw.supp -- .\task2.exe 0.001
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <ctype.h>

typedef enum {
    STATUS_OK,
    STATUS_NULL_POINTER,
    STATUS_INVALID_NUMBER,
    STATUS_INVALID_EPSILON,
    STATUS_INVALID_INTERVAL,
    STATUS_ITERATION_LIMIT,
    STATUS_MEMORY_ERROR
} Status;

typedef double (*Equation)(double x, double param);
typedef Status (*Method)(double eps, double *result);

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

Status bisection(Equation f, const double param, double a, double b, const double eps, double *root) {
    double fa;
    double mid;
    double fmid;
    const int max_steps = 200;
    int i;
    if (f == NULL || root == NULL) {
        return STATUS_NULL_POINTER;
    }
    if (check_eps(eps) != STATUS_OK) {
        return STATUS_INVALID_EPSILON;
    }
    if (a > b - eps) {
        return STATUS_INVALID_INTERVAL;
    }
    fa = f(a, param);
    if (fa * f(b, param) > 0.0) {
        return STATUS_INVALID_INTERVAL;
    }
    for (i = 0; i < max_steps && b - a > eps; i++) {
        mid = (a + b) / 2.0;
        fmid = f(mid, param);
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

double equation_e(double x, double param) {
    (void)param;
    return log(x) - 1.0;
}

double equation_pi(double x, double param) {
    (void)param;
    return cos(x / 2.0);
}

double equation_ln2(double x, double param) {
    (void)param;
    return exp(x) - 2.0;
}

double equation_sqrt2(double x, double param) {
    (void)param;
    return x * x - 2.0;
}

double equation_gamma(double x, double param) {
    return exp(-x) - param;
}

Status e_limit(double eps, double *result) {
    double n = 1.0;
    double prev = 2.0;
    double cur;
    const int max_doublings = 40;
    int i;
    if (result == NULL) {
        return STATUS_NULL_POINTER;
    }
    if (check_eps(eps) != STATUS_OK) {
        return STATUS_INVALID_EPSILON;
    }
    for (i = 0; i < max_doublings; i++) {
        n *= 2.0;
        cur = pow(1.0 + 1.0 / n, n);
        if (fabs(cur - prev) < eps) {
            *result = cur;
            return STATUS_OK;
        }
        prev = cur;
    }
    *result = prev;
    return STATUS_ITERATION_LIMIT;
}

Status e_series(double eps, double *result) {
    double sum = 1.0;
    double term = 1.0;
    long n;
    const long max_terms = 1000000000L;
    if (result == NULL) {
        return STATUS_NULL_POINTER;
    }
    if (check_eps(eps) != STATUS_OK) {
        return STATUS_INVALID_EPSILON;
    }
    for (n = 1; n < max_terms; n++) {
        term /= (double)n;
        sum += term;
        if (term < eps) {
            *result = sum;
            return STATUS_OK;
        }
    }
    *result = sum;
    return STATUS_ITERATION_LIMIT;
}

Status e_equation(double eps, double *result) {
    if (result == NULL) {
        return STATUS_NULL_POINTER;
    }
    return bisection(equation_e, 0.0, 2.0, 3.0, eps, result);
}

Status pi_limit(double eps, double *result) {
    double value = 4.0;
    double prev = value;
    double n;
    double next_check = 2.0;
    const long max_terms = 1000000000L;
    if (result == NULL) {
        return STATUS_NULL_POINTER;
    }
    if (check_eps(eps) != STATUS_OK) {
        return STATUS_INVALID_EPSILON;
    }
    for (n = 1.0; n < (double)max_terms; n += 1.0) {
        value *= 4.0 * n * (n + 1.0) / ((2.0 * n + 1.0) * (2.0 * n + 1.0));
        if (n + 1.0 == next_check) {
            if (fabs(value - prev) < eps) {
                *result = value;
                return STATUS_OK;
            }
            prev = value;
            next_check *= 2.0;
        }
    }
    *result = value;
    return STATUS_ITERATION_LIMIT;
}

Status pi_series(double eps, double *result) {
    double sum = 0.0;
    double term;
    long n;
    const long max_terms = 1000000000L;
    if (result == NULL) {
        return STATUS_NULL_POINTER;
    }
    if (check_eps(eps) != STATUS_OK) {
        return STATUS_INVALID_EPSILON;
    }
    for (n = 1; n < max_terms; n++) {
        term = 4.0 / (2.0 * (double)n - 1.0);
        if (term < eps) {
            *result = sum;
            return STATUS_OK;
        }
        sum += n % 2 == 1 ? term : -term;
    }
    *result = sum;
    return STATUS_ITERATION_LIMIT;
}

Status pi_equation(double eps, double *result) {
    if (result == NULL) {
        return STATUS_NULL_POINTER;
    }
    return bisection(equation_pi, 0.0, 2.0, 4.0, eps, result);
}

Status ln2_limit(double eps, double *result) {
    double n = 1.0;
    double prev = 1.0;
    double cur;
    const int max_doublings = 40;
    int i;
    if (result == NULL) {
        return STATUS_NULL_POINTER;
    }
    if (check_eps(eps) != STATUS_OK) {
        return STATUS_INVALID_EPSILON;
    }
    for (i = 0; i < max_doublings; i++) {
        n *= 2.0;
        cur = n * (pow(2.0, 1.0 / n) - 1.0);
        if (fabs(cur - prev) < eps) {
            *result = cur;
            return STATUS_OK;
        }
        prev = cur;
    }
    *result = prev;
    return STATUS_ITERATION_LIMIT;
}

Status ln2_series(double eps, double *result) {
    double sum = 0.0;
    double term;
    long n;
    const long max_terms = 1000000000L;
    if (result == NULL) {
        return STATUS_NULL_POINTER;
    }
    if (check_eps(eps) != STATUS_OK) {
        return STATUS_INVALID_EPSILON;
    }
    for (n = 1; n < max_terms; n++) {
        term = 1.0 / (double)n;
        if (term < eps) {
            *result = sum;
            return STATUS_OK;
        }
        sum += n % 2 == 1 ? term : -term;
    }
    *result = sum;
    return STATUS_ITERATION_LIMIT;
}

Status ln2_equation(double eps, double *result) {
    if (result == NULL) {
        return STATUS_NULL_POINTER;
    }
    return bisection(equation_ln2, 0.0, 0.0, 1.0, eps, result);
}

Status sqrt2_limit(double eps, double *result) {
    double x = -0.5;
    double next;
    long i;
    const long max_terms = 1000000000L;
    if (result == NULL) {
        return STATUS_NULL_POINTER;
    }
    if (check_eps(eps) != STATUS_OK) {
        return STATUS_INVALID_EPSILON;
    }
    for (i = 0; i < max_terms; i++) {
        next = x - x * x / 2.0 + 1.0;
        if (fabs(next - x) < eps) {
            *result = next;
            return STATUS_OK;
        }
        x = next;
    }
    *result = x;
    return STATUS_ITERATION_LIMIT;
}

Status sqrt2_product(double eps, double *result) {
    double product = 1.0;
    double exponent = 0.25;
    double term;
    int k;
    if (result == NULL) {
        return STATUS_NULL_POINTER;
    }
    if (check_eps(eps) != STATUS_OK) {
        return STATUS_INVALID_EPSILON;
    }
    for (k = 2; k < 1000; k++) {
        term = pow(2.0, exponent);
        product *= term;
        if (term - 1.0 < eps) {
            *result = product;
            return STATUS_OK;
        }
        exponent /= 2.0;
    }
    *result = product;
    return STATUS_ITERATION_LIMIT;
}

Status sqrt2_equation(double eps, double *result) {
    if (result == NULL) {
        return STATUS_NULL_POINTER;
    }
    return bisection(equation_sqrt2, 0.0, 1.0, 2.0, eps, result);
}

Status gamma_limit(double eps, double *result) {
    long double prev = 0.0L;
    long double sum = 0.0L;
    long double binomial;
    long double log_factorial;
    const int max_m = 50;
    int m;
    int k;
    if (result == NULL) {
        return STATUS_NULL_POINTER;
    }
    if (check_eps(eps) != STATUS_OK) {
        return STATUS_INVALID_EPSILON;
    }
    for (m = 1; m <= max_m; m++) {
        sum = 0.0L;
        binomial = 1.0L;
        log_factorial = 0.0L;
        for (k = 1; k <= m; k++) {
            binomial = binomial * (long double)(m - k + 1) / (long double)k;
            log_factorial += logl((long double)k);
            sum += (k % 2 == 0 ? 1.0L : -1.0L) * binomial / (long double)k * log_factorial;
        }
        if (m > 1 && fabsl(sum - prev) < (long double)eps) {
            *result = (double)sum;
            return STATUS_OK;
        }
        prev = sum;
    }
    *result = (double)sum;
    return STATUS_ITERATION_LIMIT;
}

Status gamma_series(double eps, double *result) {
    double pi = acos(-1.0);
    double sum = -pi * pi / 6.0;
    double block;
    const long max_blocks = 3000;
    long n;
    long k;
    if (result == NULL) {
        return STATUS_NULL_POINTER;
    }
    if (check_eps(eps) != STATUS_OK) {
        return STATUS_INVALID_EPSILON;
    }
    for (n = 1; n <= max_blocks; n++) {
        block = 0.0;
        for (k = n * n > 2 ? n * n : 2; k < (n + 1) * (n + 1); k++) {
            block += 1.0 / (double)(n * n) - 1.0 / (double)k;
        }
        sum += block;
        if (1.0 / ((double)n * (double)n) < eps) {
            *result = sum + 2.0 / ((double)n + 0.5);
            return STATUS_OK;
        }
    }
    *result = sum + 2.0 / ((double)max_blocks + 0.5);
    return STATUS_ITERATION_LIMIT;
}

Status mertens_limit(double eps, double *result) {
    char *is_composite;
    double product = 1.0;
    double needed;
    const long max_prime_limit = 20000000L;
    long limit;
    long t;
    long j;
    Status status = STATUS_OK;
    if (result == NULL) {
        return STATUS_NULL_POINTER;
    }
    if (check_eps(eps) != STATUS_OK) {
        return STATUS_INVALID_EPSILON;
    }
    needed = (0.1 / eps) * (0.1 / eps);
    if (needed < 100.0) {
        needed = 100.0;
    }
    if (needed > (double)max_prime_limit) {
        limit = max_prime_limit;
        status = STATUS_ITERATION_LIMIT;
    } else {
        limit = (long)needed;
    }
    is_composite = (char *)calloc((size_t)limit + 1, sizeof(char));
    if (is_composite == NULL) {
        return STATUS_MEMORY_ERROR;
    }
    for (t = 2; t <= limit; t++) {
        if (!is_composite[t]) {
            product *= (double)(t - 1) / (double)t;
            if (t <= limit / t) {
                for (j = t * t; j <= limit; j += t) {
                    is_composite[j] = 1;
                }
            }
        }
    }
    free(is_composite);
    *result = log((double)limit) * product;
    return status;
}

Status gamma_equation(double eps, double *result) {
    double limit;
    Status limit_status;
    Status status;
    if (result == NULL) {
        return STATUS_NULL_POINTER;
    }
    if (check_eps(eps) != STATUS_OK) {
        return STATUS_INVALID_EPSILON;
    }
    limit_status = mertens_limit(eps, &limit);
    if (limit_status != STATUS_OK && limit_status != STATUS_ITERATION_LIMIT) {
        return limit_status;
    }
    status = bisection(equation_gamma, limit, 0.0, 1.0, eps, result);
    if (status == STATUS_OK) {
        return limit_status;
    }
    return status;
}

int main(int argc, char *argv[]) {
    const char *names[5] = {"e", "pi", "ln 2", "sqrt 2", "gamma"};
    const char *method_names[3] = {"предел", "ряд", "уравнение"};
    Method methods[5][3] = {
        {e_limit, e_series, e_equation},
        {pi_limit, pi_series, pi_equation},
        {ln2_limit, ln2_series, ln2_equation},
        {sqrt2_limit, sqrt2_product, sqrt2_equation},
        {gamma_limit, gamma_series, gamma_equation}
    };
    double eps;
    double value;
    int i;
    int j;
    Status status;

    if (argc != 2) {
        printf("Использование: %s <эпсилон>\n", argv[0]);
        return 1;
    }
    if (parse_double(argv[1], &eps) != STATUS_OK) {
        printf("Ошибка: эпсилон должен быть вещественным числом\n");
        return 1;
    }
    if (check_eps(eps) != STATUS_OK) {
        printf("Ошибка: эпсилон должен быть в интервале (0, 1)\n");
        return 1;
    }

    for (i = 0; i < 5; i++) {
        printf("%s:\n", names[i]);
        for (j = 0; j < 3; j++) {
            status = methods[i][j](eps, &value);
            switch (status) {
                case STATUS_OK:
                    printf("  %s: %.15f\n", method_names[j], value);
                    break;
                case STATUS_ITERATION_LIMIT:
                    printf("  %s: %.15f (достигнут предел итераций, точность не гарантируется)\n", method_names[j], value);
                    break;
                case STATUS_MEMORY_ERROR:
                    printf("  %s: ошибка: не удалось выделить память\n", method_names[j]);
                    break;
                case STATUS_INVALID_INTERVAL:
                    printf("  %s: ошибка: на интервале нет корня\n", method_names[j]);
                    break;
                default:
                    printf("  %s: ошибка вычисления\n", method_names[j]);
                    break;
            }
        }
    }
    return 0;
}
