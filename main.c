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
#define ERROR_WRONG_OPERATORS_SEQUENCE 11


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

int read_value(double * value, const char ** str, char * cur_operator, char * has_value){
    char * endptr = 0;
    *value = strtod(*str, &endptr);

    if (*value ==  HUGE_VAL) {
        fprintf(stderr, "очень длинное число: '^%s'\n", *str);
        return ERROR_HUGE_VAL;
    }
    *has_value = *str != endptr;
    *str = endptr;
    *cur_operator = *endptr;
    return NO_ERROR;
}

int _decode_expression(const char ** str, double * res, char last_operator, char bracket) {
    int error = NO_ERROR;
    double exp_res = 0.0;
    char compute = 0;
    char cur_operator = 0;
    char has_value = 0;
    double value;

    while(1) {
        if ((error = read_value(&value, str, &cur_operator, &has_value)))
            return error;

        //+- or */
        if (!is_ll(cur_operator) || !is_hl(cur_operator)) {
            if (cur_operator == last_operator) {
                *res = value;
                return NO_ERROR;
            } else if (!has_value && !is_ll(last_operator) && !is_hl(cur_operator)) {
                fprintf(stderr, "не верная последовательность операторов\n");
                return ERROR_WRONG_OPERATORS_SEQUENCE;
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
                fprintf(stderr, "пустое выражение: '^%s'\n", *str);
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
            fprintf(stderr, "неизвестный оператор: '^%s'\n", *str);
            return ERROR_EMPTY_EXPRESSION;
        }
    }

    if (bracket){
        fprintf(stderr, "нет завершающей скобки: '^%s'\n", *str);
        return ERROR_NO_END_BRACKET;
    } else if (!has_value && !compute) {
        fprintf(stderr, "пустое выражение: '^%s'\n", *str);
        return ERROR_EMPTY_EXPRESSION;
    }

    if (!compute)
        *res = value;
    else if (has_value)
        *res = compute_operation('*', *res, value);
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
    *res = 0.0;
    return _decode_expression((const char **)&pointer, res, 0, 0);
}



typedef struct _TestData {
    const char * string;
    int error;
    double res;
} TestData;

int tests() {
    TestData test_list [] = {
        {"2(2+3)2", NO_ERROR, 20.0},
        {"12+*45", ERROR_WRONG_OPERATORS_SEQUENCE, 0},
        {"1 * (1*2*(5 + 6))", NO_ERROR, 22.0},
        {"1 * (1+2)", NO_ERROR, 3},
        {"()", ERROR_EMPTY_EXPRESSION, 0},
        {"(1)", NO_ERROR, 1},
        {"(1) 1", NO_ERROR, 1},
        {"(1) (1)", NO_ERROR, 1},
        {"(1) * 1", NO_ERROR, 1},
        {"(1+2)", NO_ERROR, 2},
        {"(1+2*3)", NO_ERROR, 7.0},
        {"(1+2*(3+70))", NO_ERROR, 147},
    };
    double res;
    int error = 0;
    int i = 0;
    for (; i < sizeof(test_list) / sizeof(TestData); i++) {
        if (test_list[i].error != (error = decode_expression(test_list[i].string, &res))) {
            printf("test: %d \'%s\' error: %d\n", i, test_list[i].string, error);
            return error;
        } else {
            if (res == test_list[i].res) {
                printf("test: %d \'%s\' complete res: %f\n", i, test_list[i].string, res);
            } else {
                printf("test: %d \'%s\' res: %f != test.res: %f\n", i, test_list[i].string, res, test_list[i].res);
            }
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
