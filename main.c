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

void init_tocken_array(struct TockenArray * array) {
    array->buffer = NULL;
    array->size = 0;
}

void free_tocken_array(struct TockenArray * array) {
    if (array->buffer) {
        free(array->buffer);
    }
    array->buffer = NULL;
    array->size = 0;
}

int add_tocken(struct TockenArray * array, struct Tocken * data){
    struct Tocken * new_buffer = (struct Tocken *)realloc(
        array->buffer, (array->size + 1) * sizeof(struct Tocken));

    if (new_buffer == NULL){
        fprintf(stderr, "cannot allocate Tockens buffer\n");
        return 1;
    }

    new_buffer[array->size++] = *data;
    array->buffer = new_buffer;
    int i;
    char operator;

    /*
    for (i = 0; i < array->size; i++) {
        if (array->buffer[i].type == VALUE) {
            printf("%f ", array->buffer[i].data.value);
        } else {
            operator = array->buffer[i].data.operator;
            printf("%c ", array->buffer[i].data.operator);
        }
    }
    printf("\n");
    */
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
        struct TockenArray * out_tockens,
        char operator) {

    if (!in_tockens->size) {
        fprintf(stderr, "empty tockens array\n");
        return 1;
    }

    if (in_tockens->size == 1) {
        return add_tocken(out_tockens, &in_tockens->buffer[0]);
    }

    free_tocken_array(out_tockens);
    init_tocken_array(out_tockens);

    int error = 0;
    int i;

    //1. выполним все операции умножения, сократим массив
    for (i = 1; i < in_tockens->size - 1; i= i + 2) {
        printf("operator: %c\n", in_tockens->buffer[i].data.operator);
        if (in_tockens->buffer[i].data.operator == operator) {
            struct Tocken tocken;
            tocken.type = VALUE;

            switch (in_tockens->buffer[i].data.operator) {
                case '*':
                    tocken.data.value = in_tockens->buffer[i - 1].data.value *
                        in_tockens->buffer[i + 1].data.value;
                    break;
                case '/':
                    tocken.data.value = in_tockens->buffer[i - 1].data.value /
                        in_tockens->buffer[i + 1].data.value;
                    break;
                case '+':
                    tocken.data.value = in_tockens->buffer[i - 1].data.value +
                        in_tockens->buffer[i + 1].data.value;
                    break;
                case '-':
                    tocken.data.value = in_tockens->buffer[i - 1].data.value +
                        in_tockens->buffer[i + 1].data.value;
                    break;
            }
            if (error = add_tocken(out_tockens, &tocken))
                return error;
        } else {
            if (error = add_tocken(out_tockens, &in_tockens->buffer[i - 1]))
                return error;

            if (error = add_tocken(out_tockens, &in_tockens->buffer[i]))
                return error;
        }
    }

    //Добавим последнее значение
    if (out_tockens->buffer[out_tockens->size - 1].type == OPERATOR) {
        if (error = add_tocken(
                out_tockens, &in_tockens->buffer[in_tockens->size - 1]))
            return error;
    }
    return error;
}

int compute_expression(struct TockenArray * tockens, double * d_res) {
    if (!tockens->size) {
        fprintf(stderr, "empty expression\n");
        return 1;
    }
    int i;
    struct TockenArray new_tockens;
    init_tocken_array(&new_tockens);
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
    if (error=reduce_tockens(tockens, &new_tockens, '*')) {
        free_tocken_array(&new_tockens);
        return error;
    }

    if (error=reduce_tockens(&new_tockens, tockens, '/')) {
        free_tocken_array(&new_tockens);
        return error;
    }

    if (error=reduce_tockens(tockens, &new_tockens, '+')){
        free_tocken_array(&new_tockens);
        return error;
    }

    if (error=reduce_tockens(&new_tockens, tockens, '-')){
        free_tocken_array(&new_tockens);
        return error;
    }

    *d_res = tockens->buffer[0].data.value;
    free_tocken_array(&new_tockens);
    return error;
}


int str_to_double(const char * start, const char * end, double * d_res){
    char * buffer = TEMP_STR_BUFFER;
    char * ukaszatel = NULL;

    //заполним вреенный буффер, исключаем пробелы
    for (; start != end && buffer != &TEMP_STR_BUFFER[MAX_VALUE_LENGTH];
            start++, buffer++) {
        if (*start != ' ') {
            *buffer=*start;
        }
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


int extract_tockens(struct TockenArray * array, const char ** start_tocken, const char * str) {
    int error = 0;
    //найден оператор
    if (!in_sequence(*str, OPERATORS, sizeof(OPERATORS))) {
        //проверка знака числа
        char * st = *start_tocken;
        if (!array->size && st == str) {
            // не знак числа
            if (in_sequence(*str, SIGNS, sizeof(SIGNS))) {
                fprintf(stderr, "wrong operator: %c\n", *str);
                return 1;
            }
            // игнорируем знак числа
            return 0;
        }
        //распарсим значение
        if (error = extract_value(array, *start_tocken, str))
            return error;
        if (error = add_operator(array, *str))
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
                //выделим число
                if (error = extract_value(&tockens, start_tocken, str)) {
                    return error;
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

    if (start_bracket != NULL) {
        error = 1;
        fprintf(stderr, "expression is not complete\n");
    }

    // вычислим выражение
    if (tockens.size) {
        if (tockens.buffer[tockens.size - 1].type == OPERATOR) {
            if (error = extract_value(&tockens, start_tocken, str)) {

            }
        }

        if (!error)
            error = compute_expression(&tockens, d_res);
    // выделим значение
    } else {
        if (!(error = extract_value(&tockens, start_tocken, str))) {
            *d_res = tockens.buffer[tockens.size - 1].data.value;
        }
    }

    free_tocken_array(&tockens);
    return error;
}

int analyze_expressions(const char * _str, double * d_res) {
    const char * str = _str;
    return _analyze_expressions(&str, NULL, d_res);
}


int main(){
    //const char * expression = "-1( -2 * 3 / [1+ 1] + 1) + 1 + {1 + 2} {12}";
    const char * expression = "-1(1+2)";
    double d_res;
    //const char * expression = "1";
    int error = analyze_expressions(expression, &d_res);

    if (!error) {
        printf("expression: %s result: %f\n", expression, d_res);
    }
    return 0;
}
