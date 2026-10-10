// .\task4.exe -i tests\in4.txt
// .\task4.exe -d tests\in4.txt
// .\task4.exe /s tests\in4.txt
// .\task4.exe -a tests\in4.txt

//.\task4.exe -nd tests\in4.txt tests\res_d.txt
//.\task4.exe -ni tests\in4.txt tests\res_i.txt
//.\task4.exe /ns tests\in4.txt tests\res_s.txt
// .\task4.exe -na tests\in4.txt tests\res_a.txt

//drmemory -suppress mingw.supp -- .\task4.exe -d tests\in4.txt
//drmemory -suppress mingw.supp -- .\task4.exe -nd tests\in4.txt нет_папки\res.txt

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum {
    STATUS_OK,
    STATUS_NULL_POINTER,
    STATUS_INVALID_FLAG,
    STATUS_MEMORY_ERROR,
    STATUS_READ_ERROR,
    STATUS_WRITE_ERROR
} Status;

int is_latin(const int c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

int is_digit(const int c) {
    return c >= '0' && c <= '9';
}

Status parse_flag(const char *flag, char *action, int *has_output) {
    const char *letter;
    if (flag == NULL || action == NULL || has_output == NULL) {
        return STATUS_NULL_POINTER;
    }
    if (flag[0] != '-' && flag[0] != '/') {
        return STATUS_INVALID_FLAG;
    }
    letter = flag + 1;
    *has_output = 0;
    if (letter[0] == 'n') {
        *has_output = 1;
        letter++;
    }
    if (letter[0] == '\0' || letter[1] != '\0') {
        return STATUS_INVALID_FLAG;
    }
    if (letter[0] != 'd' && letter[0] != 'i' && letter[0] != 's' && letter[0] != 'a') {
        return STATUS_INVALID_FLAG;
    }
    *action = letter[0];
    return STATUS_OK;
}

Status make_output_name(const char *input, char **output) {
    const char *prefix = "out_";
    const char *name = input;
    const char *p;
    size_t dir_length;
    if (input == NULL || output == NULL) {
        return STATUS_NULL_POINTER;
    }
    for (p = input; *p != '\0'; p++) {
        if (*p == '/' || *p == '\\') {
            name = p + 1;
        }
    }
    dir_length = (size_t)(name - input);
    *output = (char *)malloc(strlen(input) + strlen(prefix) + 1);
    if (*output == NULL) {
        return STATUS_MEMORY_ERROR;
    }
    memcpy(*output, input, dir_length);
    strcpy(*output + dir_length, prefix);
    strcat(*output, name);
    return STATUS_OK;
}

Status remove_digits(FILE *in, FILE *out) {
    int c;
    if (in == NULL || out == NULL) {
        return STATUS_NULL_POINTER;
    }
    while ((c = fgetc(in)) != EOF) {
        if (!is_digit(c) && fputc(c, out) == EOF) {
            return STATUS_WRITE_ERROR;
        }
    }
    return ferror(in) ? STATUS_READ_ERROR : STATUS_OK;
}

Status count_in_lines(FILE *in, FILE *out, int (*should_count)(int)) {
    int c;
    int has_chars = 0;
    long count = 0;
    if (in == NULL || out == NULL || should_count == NULL) {
        return STATUS_NULL_POINTER;
    }
    while ((c = fgetc(in)) != EOF) {
        if (c == '\n') {
            if (fprintf(out, "%ld\n", count) < 0) {
                return STATUS_WRITE_ERROR;
            }
            count = 0;
            has_chars = 0;
        } else {
            has_chars = 1;
            if (should_count(c)) {
                count++;
            }
        }
    }
    if (ferror(in)) {
        return STATUS_READ_ERROR;
    }
    if (has_chars && fprintf(out, "%ld\n", count) < 0) {
        return STATUS_WRITE_ERROR;
    }
    return STATUS_OK;
}

int is_special(const int c) {
    return !is_latin(c) && !is_digit(c) && c != ' ' && c != '\r';
}

Status replace_with_hex(FILE *in, FILE *out) {
    int c;
    int written;
    if (in == NULL || out == NULL) {
        return STATUS_NULL_POINTER;
    }
    while ((c = fgetc(in)) != EOF) {
        if (is_digit(c)) {
            written = fputc(c, out) == EOF ? -1 : 1;
        } else {
            written = fprintf(out, "%02X", (unsigned int)c);
        }
        if (written < 0) {
            return STATUS_WRITE_ERROR;
        }
    }
    return ferror(in) ? STATUS_READ_ERROR : STATUS_OK;
}

Status process_file(FILE *in, FILE *out, const char action) {
    switch (action) {
        case 'd':
            return remove_digits(in, out);
        case 'i':
            return count_in_lines(in, out, is_latin);
        case 's':
            return count_in_lines(in, out, is_special);
        case 'a':
            return replace_with_hex(in, out);
        default:
            return STATUS_INVALID_FLAG;
    }
}

int main(int argc, char *argv[]) {
    char action;
    int has_output;
    char *output_name = NULL;
    FILE *in;
    FILE *out;
    Status status;

    if (argc < 3) {
        printf("Использование: %s <флаг> <входной файл> [выходной файл]\n", argv[0]);
        printf("Флаги: -d -i -s -a, с выходным файлом: -nd -ni -ns -na\n");
        return 1;
    }

    status = parse_flag(argv[1], &action, &has_output);
    if (status != STATUS_OK) {
        printf("Ошибка: некорректный флаг %s\n", argv[1]);
        return 1;
    }

    if (has_output) {
        if (argc != 4) {
            printf("Ошибка: для флага %s нужен путь к выходному файлу\n", argv[1]);
            return 1;
        }
        output_name = (char *)malloc(strlen(argv[3]) + 1);
        if (output_name == NULL) {
            printf("Ошибка: не удалось выделить память\n");
            return 1;
        }
        strcpy(output_name, argv[3]);
    } else {
        if (argc != 3) {
            printf("Ошибка: для флага %s нужен только входной файл\n", argv[1]);
            return 1;
        }
        status = make_output_name(argv[2], &output_name);
        if (status != STATUS_OK) {
            printf("Ошибка: не удалось выделить память\n");
            return 1;
        }
    }

    if (strcmp(argv[2], output_name) == 0) {
        printf("Ошибка: входной и выходной файлы совпадают\n");
        free(output_name);
        return 1;
    }

    in = fopen(argv[2], "r");
    if (in == NULL) {
        printf("Ошибка: не удалось открыть файл %s\n", argv[2]);
        free(output_name);
        return 1;
    }
    out = fopen(output_name, "w");
    if (out == NULL) {
        printf("Ошибка: не удалось открыть файл %s\n", output_name);
        fclose(in);
        free(output_name);
        return 1;
    }

    status = process_file(in, out, action);
    fclose(in);
    if (fclose(out) != 0 && status == STATUS_OK) {
        status = STATUS_WRITE_ERROR;
    }

    switch (status) {
        case STATUS_OK:
            printf("Результат записан в файл %s\n", output_name);
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
    free(output_name);
    return status == STATUS_OK ? 0 : 1;
}
