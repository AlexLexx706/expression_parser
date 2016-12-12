#include <stdio.h>
#include <stdlib.h>

#define MAX_VALUE_LENGTH 20

const char START_BRACKET [] = {'{', '[', '('};
const char END_BRACKET [] = {'}', ']', ')'};
const char OPERATORS [] = {'+', '-', '/', '*'};
const char SIGNS [] = {'+', '-'};


char TEMP_STR_BUFFER[MAX_VALUE_LENGTH + 1];

struct Tocken{
    enum {
        VALUE,
        OPERATOR} type;

    union {
        double value;
        char operator;
    } data;
};

struct TockenArray{
    struct Tocken * buffer;
    unsigned int size;
};

void init_tocken_array(struct TockenArray * tockens) {
    tockens->buffer = NULL;
    tockens->size = 0;
}

void free_tocken_array(struct TockenArray * tockens) {
    if (tockens->buffer) {
        free(tockens->buffer);
    }
    tockens->buffer = NULL;
    tockens->size = 0;
}

int add_tocken(struct TockenArray * tockens, struct Tocken * data) {
    struct Tocken * new_buffer = (struct Tocken *)realloc(
        tockens->buffer, (tockens->size + 1) * sizeof(struct Tocken));

    if (new_buffer == NULL){
        fprintf(stderr, "cannot allocate Tockens buffer\n");
        return 1;
    }

    new_buffer[tockens->size++] = *data;
    tockens->buffer = new_buffer;
    return 0;
}

int remove_tocken(struct TockenArray * tockens, int index) {
    for (; index < tockens->size - 1; index++) {
        tockens->buffer[index] = tockens->buffer[index + 1];
    }
    tockens->size--;
    return 0;
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

int reduce_tockens(
        struct TockenArray * in_tockens,
        char operator) {

    if (!in_tockens->size) {
        fprintf(stderr, "empty tockens array\n");
        return 1;
    }
    int error = 0;
    int i;

    if (in_tockens->size == 1) {
        return 0;
    }

    //1. выполним все операции умножения, сократим массив
    for (i = 1; i < in_tockens->size - 1; i= i + 2) {
        if (in_tockens->buffer[i].data.operator == operator) {
            switch (in_tockens->buffer[i].data.operator) {
                case '*':
                    in_tockens->buffer[i - 1].data.value = in_tockens->buffer[i - 1].data.value *
                        in_tockens->buffer[i + 1].data.value;
                    break;
                case '/':
                    if (in_tockens->buffer[i + 1].data.value == 0.0) {
                        fprintf(stderr, "devide by zerro\n");
                        return 1;
                    }
                    in_tockens->buffer[i - 1].data.value = in_tockens->buffer[i - 1].data.value /
                        in_tockens->buffer[i + 1].data.value;
                    break;
                case '+':
                    in_tockens->buffer[i - 1].data.value = in_tockens->buffer[i - 1].data.value +
                        in_tockens->buffer[i + 1].data.value;
                    break;
                case '-':
                    in_tockens->buffer[i - 1].data.value = in_tockens->buffer[i - 1].data.value +
                        in_tockens->buffer[i + 1].data.value;
                    break;
            }
            remove_tocken(in_tockens, i);
            remove_tocken(in_tockens, i);
        }
    }
    return error;
}

int compute_expression(struct TockenArray * tockens, double * d_res) {
    if (!tockens->size) {
        fprintf(stderr, "empty expression\n");
        return 1;
    }
    int i;
    int error;

    for (i = 0; i < tockens->size; i++) {
        if (tockens->buffer[i].type == VALUE) {
            printf("%f ", tockens->buffer[i].data.value);
        } else {
            printf("%c ", tockens->buffer[i].data.operator);
        }
    }
    printf("\n");

    //1. выполним все операции умножения, сократим массив
    if (error=reduce_tockens(tockens, '*')) {
        return error;
    }

    if (error=reduce_tockens(tockens, '/')) {
        return error;
    }

    if (error=reduce_tockens(tockens, '+')){
        return error;
    }

    if (error=reduce_tockens(tockens, '-')){
        return error;
    }

    *d_res = tockens->buffer[0].data.value;
    //printf("res: %f\n", *d_res);
    return error;
}


int str_to_double(const char * start, const char * end, double * d_res){
    char * buffer = TEMP_STR_BUFFER;
    char * ukaszatel = NULL;

    //заполним вреенный буффер, исключаем пробелы
    for (; start != end && buffer != &TEMP_STR_BUFFER[MAX_VALUE_LENGTH];
            start++) {
        if (*start != ' ') {
            *buffer=*start;
            buffer++;
        }
    }
    if (buffer == TEMP_STR_BUFFER) {
        fprintf(stderr, "empty value\n");
        return 1;
    }
    *buffer = 0;

    // очень длинная строка
    if (buffer == &TEMP_STR_BUFFER[MAX_VALUE_LENGTH]){
        fprintf(stderr, "string: %s len > %d\n", buffer, MAX_VALUE_LENGTH);
        return 2;
    }

    *d_res = strtod(TEMP_STR_BUFFER, &ukaszatel);
    if (*ukaszatel != 0){
        fprintf(stderr, "cannot convert: %s to double\n", TEMP_STR_BUFFER);
        return 1;
    }
    return 0;
}


int extract_value(struct TockenArray * array, const char * start_tocken, const char * str) {
    int error = 0;
    //распарсим значение

    double d_res;
    if (error = str_to_double(start_tocken, str, &d_res))
        return error;

    //Добавили значение
    struct Tocken tocken = {VALUE, d_res};
    return add_tocken(array, &tocken);
}

int add_operator(struct TockenArray * array, char operator) {
    struct Tocken tocken;
    tocken.type = OPERATOR;
    tocken.data.operator=operator;
    return add_tocken(array, &tocken);
}


int extract_tockens(struct TockenArray * tockens, const char ** start_tocken, const char * str) {
    int error = 0;
    //найден оператор
    if (!in_sequence(*str, OPERATORS, sizeof(OPERATORS))) {
        //проверка знака числа
        char * st = *start_tocken;
        if (!tockens->size && st == str) {
            // не знак числа
            if (in_sequence(*str, SIGNS, sizeof(SIGNS))) {
                fprintf(stderr, "wrong operator: %c\n", *str);
                return 1;
            }
            // игнорируем знак числа
            return 0;
        }
        //распарсим значение
        if (!tockens->size || tockens->buffer[tockens->size -1].type == OPERATOR) {
            if (error = extract_value(tockens, *start_tocken, str))
                return error;
        }

        if (error = add_operator(tockens, *str))
            return error;
        *start_tocken = NULL;
    }
    return error;
}

int check_empty_multiply_operator(
        struct TockenArray * tockens, const char * start_tocken,
        const char * str) {
    int error = 0;

    // умножения на число
    if (!tockens->size) {
        //выделим значение
        if (error = extract_value(tockens, start_tocken, str))
            return error;
        if (error = add_operator(tockens, '*'))
            return error;
    //Добавим оператор
    } else if (tockens->buffer[tockens->size - 1].type == VALUE) {
        struct Tocken tocken = {OPERATOR, '*'};
        if (error = add_tocken(tockens, &tocken))
            return error;
    }
    return error;
}

int check_end_bracket(char start_bracket, char end_bracket) {
    int i = 0;
    for (; i < sizeof(START_BRACKET); i++) {
        if (start_bracket == START_BRACKET[i]){
            if (END_BRACKET[i] != end_bracket) {
                fprintf(stderr, "wrong end '%c' bracket\n", end_bracket);
                return 1;
            }
            return 0;
        }
    }
    fprintf(stderr, "wrong start bracket: '%c'\n", start_bracket);
    return 1;
}
int _analyze_expressions(const char ** in_str, const char * start_bracket, double * d_res) {
    const char  * str = *in_str;
    const char * start_tocken = str;
    struct TockenArray tockens;
    int error=0;
    *d_res = 0.0;
    init_tocken_array(&tockens);
    while (*str != 0) {
        //поиск конца
        if (start_bracket) {
            // найдено начало нового выражения
            if (!in_sequence(*str, START_BRACKET, sizeof(START_BRACKET))) {
                // умножения на число
                if (error = check_empty_multiply_operator(
                        &tockens, start_tocken, str)) {
                    return error;
                }

                (*in_str)++;
                // вычислим выражение
                if (error = _analyze_expressions(in_str, str, d_res))
                    break;
                 // добавим токен в выражение
                struct Tocken tocken = {VALUE, *d_res};

                if (error = add_tocken(&tockens, &tocken))
                    break;
                start_tocken = NULL;
            //окончание выражения
            } else if (!in_sequence(*str, END_BRACKET, sizeof(END_BRACKET))) {
                if (error = check_end_bracket(*start_bracket, *str))
                    break;
                //выделим число
                if (!tockens.size || tockens.buffer[tockens.size - 1].type != VALUE) {
                    if (error = extract_value(&tockens, start_tocken, str)) {
                        return error;
                    }
                }
                //вычислим выражение
                error = compute_expression(&tockens, d_res);
                free_tocken_array(&tockens);
                return 0;
            // выделение значений и операторов
            } else {
                if (error=extract_tockens(&tockens, &start_tocken, str))
                    break;
            }
        // поиск начала выражения
        } else if (!in_sequence(*str, START_BRACKET, sizeof(START_BRACKET))) {
            start_bracket = str;

            if (error = check_empty_multiply_operator(
                    &tockens, start_tocken, str)) {
                return error;
            }

            (*in_str)++;
            // вычислим выражение
            if (error = _analyze_expressions(in_str, str, d_res))
                break;
            start_bracket = NULL;

            // добавим токен в выражение
            struct Tocken tocken = {VALUE, *d_res};

            if (error = add_tocken(&tockens, &tocken))
                break;
            start_tocken = NULL;
        // выделение значений и операторов
        } else {
            if (error=extract_tockens(&tockens, &start_tocken, str))
                break;
        }

        str = ++(*in_str);

        if (start_tocken == NULL) {
            start_tocken = str;
        }
    }

    if (!error) {
        if (start_bracket != NULL) {
            error = 1;
            fprintf(stderr, "expression is not complete\n");
        }

        // вычислим выражение
        if (tockens.size) {
            if (tockens.buffer[tockens.size - 1].type == OPERATOR) {
                error = extract_value(&tockens, start_tocken, str);
            }

            if (!error)
                error = compute_expression(&tockens, d_res);
        // выделим значение
        } else {
            if (!(error = extract_value(&tockens, start_tocken, str))) {
                *d_res = tockens.buffer[tockens.size - 1].data.value;
            }
        }
    }

    free_tocken_array(&tockens);
    return error;
}

int analyze_expressions(const char * _str, double * d_res) {
    const char * str = _str;
    return _analyze_expressions(&str, NULL, d_res);
}


int main() {
    const char * expression = "1+(1/(1*0.78))+2";
    //const char * expression = "1 + {}";
    double d_res;
    char temp_buffer[120];
    printf("inpit expression: ");
    if (!gets(temp_buffer)) {
        fprintf(stderr, "cannot read line\n");
        return 1;
    }
    //const char * expression = "1";
    int error = analyze_expressions(temp_buffer, &d_res);
    //int error = analyze_expressions(expression, &d_res);

    if (!error) {
        printf("expression: %s result: %f\n", temp_buffer, d_res);
        //printf("expression: %s result: %f\n", expression, d_res);
    }
    return 0;
}
