#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define TEMP_BUFFER_SIZE 200
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
#define ERROR_BAD_PARAMS 10


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

double compute_operation(char expression_operator, double left, double right){
    switch (expression_operator){
        case '+':
            return left + right;
        case '-':
            return left - right;
        case '*':
            return left * right;
        case '/':
            return left / right;
    }
    return 0.0;
}

int _decode_expression(const char ** str, double * res, char last_operator, char bracket) {
    const char * start = *str;
    int error = NO_ERROR;
    double exp_res = 0.0;
    char compute = 0;
    char * endptr = 0;
    char cur_operator = 0;
    char has_value = 0;
    double value;

    for(;;) {
        value = strtod(*str, &endptr);

        if (value ==  HUGE_VAL) {
            fprintf(stderr, "очень длинное число: '^%s'\n", start);
            return ERROR_HUGE_VAL;
        }
        has_value = *str != endptr;
        *str = endptr;
        cur_operator = *endptr;

        //+-
        if (!is_ll(cur_operator)) {
            if (!is_ll(last_operator)) {
                *res = value;
                return NO_ERROR;
            }
            (*str)++;
            if ((error = _decode_expression(str, &exp_res, cur_operator, bracket)))
                return error;

            *res = compute_operation(cur_operator, has_value ? value : *res, exp_res);
            if (!is_sb(last_operator) && !is_eb(**str)) {
                (*str)++;
                return NO_ERROR;
            }
            compute = 1;
        //*/
        } else if (!is_hl(cur_operator)) {
            if (!is_hl(last_operator)) {
                *res = value;
                return NO_ERROR;
            }
            (*str)++;
            if ((error = _decode_expression(str, &exp_res, cur_operator, bracket)))
                return error;
            *res = compute_operation(cur_operator, has_value ? value : *res, exp_res);

            if (!is_sb(last_operator) && !is_eb(**str)) {
                (*str)++;
                return NO_ERROR;
            }
            compute = 1;
        // {
        } else if (!is_sb(cur_operator)) {
            (*str)++;

            if ((error = _decode_expression(str, &exp_res, cur_operator, cur_operator)))
                return error;

            if (has_value || compute) {
                *res = compute_operation('*', has_value ? value : *res, exp_res);
            } else
                *res = exp_res;
            compute = 1;
        // }
        } else if (!is_eb(cur_operator)) {
            if ((error = check_end_bracket(bracket, *str)))
                return error;

            if (!compute && !has_value) {
                fprintf(stderr, "пустое выражение: '^%s'\n", start);
                return ERROR_EMPTY_EXPRESSION;
            }
            if (has_value)
                *res = value;

            if (!is_sb(last_operator))
                (*str)++;

            return NO_ERROR;
        } else if (**str == 0 || **str == '\n')
            break;
        else {
            fprintf(stderr, "неизвестный оператор: '^%s'\n", start);
            return ERROR_EMPTY_EXPRESSION;
        }

    }

    if (bracket){
        fprintf(stderr, "нет завершающей скобки: '^%s'\n", start);
        return ERROR_NO_END_BRACKET;
    } else if (!has_value && !compute) {
        fprintf(stderr, "пустое выражение: '^%s'\n", start);
        return ERROR_EMPTY_EXPRESSION;
    }

    if (!compute)
        *res = value;
    return NO_ERROR;
}

int decode_expression(const char * str, double * res) {
    if (str == NULL || res == NULL){
        fprintf(stderr, "плохие параметры\n");
        return ERROR_BAD_PARAMS;
    }
    char temp_buffer[TEMP_BUFFER_SIZE + 1];
    char * pointer = temp_buffer;

    for (; pointer != &temp_buffer[TEMP_BUFFER_SIZE - 1] && *str != 0; str++) {
        if (*str != ' ' && *str != '\t') {
            (*pointer) = (*str);
            pointer++;
        }
    }

    if (pointer == &temp_buffer[TEMP_BUFFER_SIZE - 1]) {
        fprintf(stderr, "размер строки > %d\n", TEMP_BUFFER_SIZE);
        return ERROR_BAD_PARAMS;
    }
    *pointer = 0;
    pointer = temp_buffer;
    return _decode_expression(&pointer, res, 0, 0);
}



typedef struct _TestData {
    const char * string;
    int res;
} TestData;

int tests() {
    TestData test_list [] = {
        {"1 * (1*2*(5 + 6))", NO_ERROR},
        {"1 * (1+2)", NO_ERROR},
        {"()", ERROR_EMPTY_EXPRESSION},
        {"(1)", NO_ERROR},
        {"(1) 1", NO_ERROR},
        {"(1) (1)", NO_ERROR},
        {"(1) * 1", NO_ERROR},
        {"(1+2)", NO_ERROR},
        {"(1+2*3)", NO_ERROR},
        {"(1+2*(3+70))", NO_ERROR},
    };
    double res;
    int error = 0;
    int i = 0;
    for (; i < sizeof(test_list) / sizeof(TestData); i++) {
        if (test_list[i].res != (error = decode_expression(test_list[i].string, &res))) {
            printf("test: %d \'%s\' error: %d\n", i, test_list[i].string, error);
            return error;
        } else {
            printf("test: %d \'%s\' complete res: %f\n", i, test_list[i].string, res);
        }
    }
    return error;
}

int main() {
    double res;
    char buffer[120];
    char * str = buffer;
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

        error = decode_expression(buffer, &res);
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
