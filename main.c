#include <stdio.h>
#include <stdlib.h>

#define MAX_VALUE_LENGTH 20

const char START_BRACKET [] = {'{', '[', '('};
const char END_BRACKET [] = {'}', ']', ')'};
const char LL_OPERATORS [] = {'-', '+'};
const char HL_OPERATORS [] = {'/', '*'};

int check_end_bracket(char start_bracket, char end_backet) {
    int i = 0;
    for (; i < sizeof(START_BRACKET); i++) {
        if (START_BRACKET[i] == start_bracket) {
            if (END_BRACKET[i] != end_backet) {
                fprintf(stderr, "wrong end backet: %c", end_backet);
            }
            return 0;
        }
    }
    fprintf(stderr, "compare no bracket start:%c, end: %c", start_bracket, end_backet);
    return 2;
}

int in_sequence(char ch, const char * sequence, unsigned int size) {
    int i = 0;

    for (; i < size; i++) {
        if (ch == sequence[i]){
            return 0;
        }
    }
    return 1;
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



int str_to_double(const char * start, const char * end, double * d_res){
    char buffer[MAX_VALUE_LENGTH + 1];
    char * ch = buffer;

    //заполним вреенный буффер, исключаем пробелы
    for (; start != end && ch != &buffer[MAX_VALUE_LENGTH];
            start++) {
        if (*start != ' ') {
            *ch = *start;
            ch++;
        }
    }
    if (ch == buffer) {
        *d_res = 0.0;
        return 1;
    }

    *ch = 0;

    // очень длинная строка
    if (ch == &buffer[MAX_VALUE_LENGTH]){
        fprintf(stderr, "string: %s len > %d\n", buffer, MAX_VALUE_LENGTH);
        return 2;
    }

    char * ukaszatel = NULL;
    *d_res = strtod(buffer, &ukaszatel);

    if (*ukaszatel != 0){
        fprintf(stderr, "cannot convert: %s to double\n", buffer);
        return 1;
    }
    return 0;
}

int decode_expression(const char * str, double * res, char oper, int * step, char bracket) {
    const char * start = str;
    int error = 0;
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
                return 0;
            }

            if ((error = decode_expression(str + 1, &exp_res, *str, &exp_step, bracket)))
                return error;

            *res = *str == '+' ? *res + exp_res : *res - exp_res;
            str += exp_step;
            compute = 1;
            bracket = 0;
        //*/
        } else if (!is_hl(*str)) {
            if ((error = str_to_double(start, str, res)))
                return error;

            if (!is_hl(oper)) {
                *step = str - start;
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
                error = str_to_double(start, str, res);

                if (error == 0)
                    oper = '*';
                else if (error > 1)
                    return error;
            }

            if ((error = decode_expression(str + 1, &exp_res, 0, &exp_step, *str)))
                return error;

            *res = *str == '*' ? *res * exp_res : exp_res;
            str += exp_step;
            compute = 1;
        // }
        } else if (!is_eb(*str)) {
            if ((error = check_end_bracket(bracket, *str)))
                return error;
            if (!compute && str != start) {
                if ((error = str_to_double(start, str, res)))
                    return error;
            }
            *step = str - start + 1;
            return 0;
        }
        str++;
    }
    if (bracket){
        fprintf(stderr, "no end bracket\n");
        return 1;
    } else if (str == start) {
        fprintf(stderr, "empty expression\n");
        return 1;
    }
    *step = str - start;
    if (compute)
        return 0;

    return str_to_double(start, str, res);
}


int main() {
    double res;
    char buffer[120];
    char * str = buffer;
    int step;
    int error;
    printf("input expression: ");

    while (1) {
        if (fgets(str, (&buffer[sizeof(buffer)]) - str, stdin) == NULL) {
            fprintf(stderr, "cannot read line\n");
            return 1;
        }

        error = decode_expression(buffer, &res, 0, &step, 0);
        if (error)
            return error;
        printf("result: %f\n", res);
        printf("press enter for exit or repeat:\n");

        if ((buffer[0] = getchar()) == '\n')
            return 0;
        str = &buffer[1];
    }
    return 0;
}
