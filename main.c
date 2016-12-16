#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define MAX_VALUE_LENGTH 20
#define NO_ERROR 0
#define ERROR_EMPTY_EXPRESSION 1
#define ERROR_WRONG_END_BARACKET 2
#define ERROR_WRONG_BARACKET 3
#define ERROR_NOT_IN_SEQUENCE 4
#define ERROR_EMPTY_STRING 5
#define ERROR_STRING_TO_LONG 6
#define ERROR_CONVERT_TO_DOUBLE 7
#define ERROR_NO_END_BRACKET 8
#define ERROR_HUGE_VAL 9

const char START_BRACKET [] = {'{', '[', '('};
const char END_BRACKET [] = {'}', ']', ')'};
const char LL_OPERATORS [] = {'-', '+'};
const char HL_OPERATORS [] = {'/', '*'};

int check_end_bracket(char start_bracket, const char * end_backet) {
    int i = 0;
    for (; i < sizeof(START_BRACKET); i++) {
        if (START_BRACKET[i] == start_bracket) {
            if (END_BRACKET[i] != *end_backet) {
                fprintf(stderr, "не верная скобка: '^%s'\n", end_backet);
                return ERROR_WRONG_END_BARACKET;
            }
            return NO_ERROR;
        }
    }
    fprintf(stderr, "ошибка в логике программы, скобки не сравнимые скобки start:%c, end: %c\n", start_bracket, *end_backet);
    return ERROR_WRONG_BARACKET;
}

int in_sequence(char ch, const char * sequence, unsigned int size) {
    int i = 0;

    for (; i < size; i++) {
        if (ch == sequence[i]){
            return NO_ERROR;
        }
    }
    return ERROR_NOT_IN_SEQUENCE;
}

int is_ll(char ch){
    return in_sequence(ch, LL_OPERATORS, sizeof(LL_OPERATORS));
}

int is_hl(char ch){
    return in_sequence(ch, HL_OPERATORS, sizeof(HL_OPERATORS));
}

int is_sb(char ch){
    return in_sequence(ch, START_BRACKET, sizeof(START_BRACKET));
}

int is_eb(char ch){
    return in_sequence(ch, END_BRACKET, sizeof(END_BRACKET));
}



int str_to_double(const char * start, const char * end, double * d_res) {
    char * endptr;
    *d_res = strtod(start, &endptr);

    if (HUGE_VAL == *d_res) {
        fprintf(stderr, "очень длинное число: '^%s'\n", start);
        return ERROR_HUGE_VAL;
    }

    if (*d_res == 0) {
        fprintf(stderr, "неудалось преобразовать в double: '^%s'\n", start);
        return ERROR_CONVERT_TO_DOUBLE;
    }
    return 0;
}

int decode_expression(const char * str, double * res, char oper, int * step, char bracket) {
    const char * start = str;
    int error = NO_ERROR;
    int exp_step = 0;
    double exp_res = 0.0;
    char compute = 0;

    while (*str != 0 && *str != '\n') {
        //+-
        if (!is_ll(*str)) {
            error = str_to_double(start, str, res);

            if (error > 1)
                return error;

            if (!is_ll(oper)) {
                *step = str - start;
                return NO_ERROR;
            }

            if ((error = decode_expression(str + 1, &exp_res, *str, &exp_step, bracket)))
                return error;

            *res = *str == '+' ? *res + exp_res : *res - exp_res;

            if (bracket) {
                *step = (str - start + 1) + exp_step;
                return NO_ERROR;
            }
            str += exp_step;
            compute = 1;
        //*/
        } else if (!is_hl(*str)) {
            if (!compute && (error = str_to_double(start, str, res)))
                return error;

            if (!is_hl(oper)) {
                *step = (str - start + 1) + exp_step;
                return 0;
            }

            if ((error = decode_expression(str + 1, &exp_res, *str, &exp_step, bracket)))
                return error;
            *res = *str == '*' ? *res * exp_res : *res / exp_res;
            *step = exp_step + (str - start) + 1;
            return 0;
        // {
        } else if (!is_sb(*str)) {
            if (compute)
                oper = '*';
            else {
                if ((error = str_to_double(start, str, res)) == NO_ERROR)
                    oper = '*';
                else if (error != ERROR_CONVERT_TO_DOUBLE)
                    return error;
            }

            if ((error = decode_expression(str + 1, &exp_res, 0, &exp_step, *str)))
                return error;

            *res = *str == '*' ? *res * exp_res : exp_res;
            str += exp_step;
            compute = 1;
        // }
        } else if (!is_eb(*str)) {
            if ((error = check_end_bracket(bracket, str)))
                return error;
            if (!compute) {
                if ((error = str_to_double(start, str, res)))
                    return error;
            }
            *step = (str - start + 1) + exp_step;
            return NO_ERROR;
        }
        str++;
    }
    if (bracket){
        fprintf(stderr, "нет завершающей скобки: '^%s'\n", start);
        return ERROR_NO_END_BRACKET;
    } else if (str == start) {
        fprintf(stderr, "пустое выражение: '^%s'\n", start);
        return ERROR_EMPTY_EXPRESSION;
    }
    *step = str - start;
    if (compute)
        return NO_ERROR;

    return str_to_double(start, str, res);
}

typedef struct _TestData {
    const char * string;
    int res;
} TestData;

int tests() {
    TestData test_list [] = {
        {"()", ERROR_CONVERT_TO_DOUBLE},
        {"(1)", NO_ERROR},
        {"(1) 1", NO_ERROR},
        {"(1) (1)", NO_ERROR},
        {"(1) * 1", NO_ERROR},
        {"(1+2)", NO_ERROR},
        {"(1+2*3)", NO_ERROR},
        {"(1+2*(3+70))", NO_ERROR},
    };
    int step;
    double res;
    int error = 0;
    int i = 0;
    for (; i < sizeof(test_list) / sizeof(TestData); i++) {
        if (test_list[i].res != (error = decode_expression(test_list[i].string, &res, 0, &step, 0))) {
            printf("test: %d \'%s\' error: %d\n", i, test_list[i].string, error);
            return error;
        } else {
            printf("test: %d \'%s\' complete\n", i, test_list[i].string);
        }
    }
    return error;
}

int main() {
    double res;
    char buffer[120];
    char * str = buffer;
    int step;
    int error;

    if ((error = tests())){
        return error;
    }

    printf("Введите выражение: ");

    while (1) {
        if (fgets(str, (&buffer[sizeof(buffer)]) - str, stdin) == NULL) {
            fprintf(stderr, "строка очень длинная\n");
            return 1;
        }

        error = decode_expression(buffer, &res, 0, &step, 0);
        if (error)
            return error;
        printf("результат: %f\n", res);
        printf("Для выхода нажмите ENTER или введите выражение:\n");

        if ((buffer[0] = getchar()) == '\n')
            return 0;
        str = &buffer[1];
    }
    return 0;
}
