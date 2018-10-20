//
//  main.cpp
//  ParsCpp
//
//  Created by Sirius on 03.10.18.
//  Copyright © 2018 Sirius. All rights reserved.
//
/*
 RunId: 2000
 Задача B-5. Парсер, логические выражения.
 
 Time limit:    14 s
 Memory limit:    64 M
 Требуется написать программу, которая способна вычислять логическе выражения.
 Допустимые выражения чувствительны к регистру и могут содержать:
 1) знаки операций 'and' - и, 'or' - или, 'not' - отрицание, 'xor' - сложение по модулю 2
 2) Скобки '(', ')'
 3) Логические значения 'True', 'False'
 4) Пустое выражение имеет значение "False"
 
 Также требуется реализовать возможность задания переменных, которые могут состоять только из маленьких букв латинского алфавита (к примеру, 'x=True').
 Объявление переменной имеет формат:
 <имя>=<True|False>; // допустимы пробелы
 
 Допускается несколько объявлений одной и той же переменной, учитывается последнее.
 Все переменные, используемые в выражении, должны быть заданы заранее. Также запрещены имена, совпадающие с ключевыми словами (and, or и т.д.).
 
 Необходимо учитывать приоритеты операций. Пробелы ничего не значат.
 
 Если выражение или объявление не удовлетворяют описанным правилам, требуется вывести в стандартный поток вывода "[error]" (без кавычек).
*/
#define _POSIX_C_SOURCE 200809L
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <ctype.h>
size_t BUFSIZE = 2;
size_t MASSIZE = 1;
int top_map = 0;
char error_flag = 0; //Флаг ошибки
char const_error = 0; //Флаг, запрещающий изменять значение error_flag

char check_mem(void);
typedef struct Var {
    char *key;
    char value;
} Variable; //Пара ключ-значение для переменной (где ключ - имя переменной)

Variable *variables; //Словарь переменных

char calculate(char *str); //Forward-declaration

char getValue(char *key) { //Функция для получения значения переменной по имени (из словаря)
    char result = 0;
    error_flag = 1; //Изначально считаем, что ошибка есть (переменная не объявлена)
    for (int i = 0; i < top_map; ++i)
        if (strcmp(variables[i].key, key) == 0) {
            result = variables[i].value;
            if (const_error == 0) {error_flag = 0;}
            break;
        }
    if (const_error == 0 && error_flag == 1) {const_error = 1;}
    free(key); //Внимание! Функция удаляет введенное имя переменной (полученное извне)
    //Вычисления продолжатся, однако на выходе произойдет проверка флага ошибки
    return result;
}
void insert_map(char *name_key, char value) { //Добавление новой переменной (изменение старой)
    //Если переменная уже есть в словаре, тогда изменяем ее значение на последнее (по условию)
    for (int i = 0; i < top_map; ++i)
        if (strcmp(variables[i].key, name_key) == 0) {
            free(variables[i].key);
            variables[i].key = name_key;
            variables[i].value = value;
            return;
        }
    if (top_map >= MASSIZE) {
        variables = (Variable*)realloc(variables, (MASSIZE *= 2) * sizeof(Variable));
        if (check_mem() == 1) error_flag = 1; //Проверка выделений памяти
    }
    variables[top_map].key = name_key;
    variables[top_map++].value = value;
}

char* remove_spaces(char *str) { //Функция удаления пробелов в инициализации переменных
    size_t len = strlen(str);
    char *newstr = (char*)malloc(len+1);
    if (check_mem() == 1) error_flag = 1; //Проверка выделений памяти
    memset(newstr, '\0', len+1);
    int top = 0;
    for (int i = 0; i < len; ++i) {
        if (str[i] != ' ') newstr[top++] = str[i];
    }
    //newstr[top] = '\0';
    return newstr;
}
int strfind(char *str, char symbol) { //Функция для нахождения позиции символа
    int pos = -1;
    for (int i = 0; i < strlen(str); ++i)
        if (str[i] == symbol) {pos = i; break;}
    return pos;
}

char* strinsert(char *str, int start, unsigned long cnt) {
   // assert(start + cnt <= strlen(str));
    assert(cnt >= 0);
    char* temp = (char*)malloc(cnt+1); //Выделение подстроки из строки
    if (check_mem() == 1) error_flag = 1; //Проверка выделений памяти
    memset(temp, '\0', cnt+1);
    for (int i = 0; i < cnt; ++i)
        temp[i] = str[start+i];
    return temp;
}

char remove_brackets(char *str) { //Удаление скобок для введенного выражения (работает рекурсивно)
    unsigned long len = strlen(str) + 1;
    unsigned int level = 0;
    char *temp = (char*)malloc(len); //Здесь накапливаем имена для операций
    if (check_mem() == 1) error_flag = 1; //Проверка выделений памяти
    memset(temp, '\0', len);
    int top = 0;
    char zn = 1;
    for (int i = 0; i < strlen(str); ++i)
    {
        if (str[i] == '(') {++level; continue;} //Повышаем уровень
        else if (str[i] == ')') {--level; continue;} //Понижаем уровень
        else if (str[i] == ' ' && level == 0) { //Пробел стирает предыдущее накопление
            free(temp);
            temp = (char*)malloc(len);
            if (check_mem() == 1) error_flag = 1; //Проверка выделений памяти
            top = 0;
            memset(temp, '\0', len);
            continue;
        }
        if (level > 0) continue; //Не рассматриваем символы в скобках
        temp[top++] = str[i];
        //temp[top] = '\0';
        if (strcmp(temp, "or") == 0 || strcmp(temp, "xor") == 0 || strcmp(temp, "and") == 0)
        {
            zn = 0; //Найденный знак на нулевом уровне запрещает нам удалять какие-либо скобки (выходим)
            break;
        }
    }
    free(temp); //Очистка памяти под временную переменную
    if (zn == 1 && str[0] == '(' && str[strlen(str) - 1] == ')')
    { //В случае успеха удаляем крайние боковые скобки
        //Запоминаем предыдущее состояние строки
        return 1;
    }

    return 0;
}
//Парсер логического выражения
char calculate(char *str) { //Функция для вычисления результата
    /*
    char f = remove_brackets(&str); //Удаляем скобки до тех пор, пока это возможно
    while (f == 1) {
        f = remove_brackets(&str);
    }
    */
    while (remove_brackets(str) == 1) {
        char *temp = strinsert(str, 1, strlen(str)-2);
        free(str);
        str = temp;
    }
    unsigned int level = 0; //Уровень глубины (0 - бесскобочный, базовый уровень)
    size_t TEMPSIZE = strlen(str) + 1;
    /* Флаг not_flag нужен для того, чтобы не взять в приоритет последний not
     * То есть для всех остальных операций мы берем последнюю
     * А для not_flag нам нужно взять первое отрицание (иначе потеряем его)
     */
    char not_flag = 0;
    int op = 10;
    /* Уровень приоритета операции для данного выражения
     * (то есть если находим более приоритетное выражение, то op меняется
     * Если выражений на уровне нет, тогда это означает, что мы попытаемся
     * получить значение из введенной строки напрямую (True/False или переменная из словаря)
     */
    int index = -1; //Начальная позиция приоритетной операции
    char *name_op; //Накопление имен операций
    char *temp = (char*)malloc(TEMPSIZE); //Тут накапливаем
    if (check_mem() == 1) error_flag = 1; //Проверка выделений памяти
    memset(temp, '\0', TEMPSIZE);
    int top = 0;
    name_op = (char*)malloc(TEMPSIZE);
    if (check_mem() == 1) error_flag = 1; //Проверка выделений памяти
    memset(name_op, '\0', TEMPSIZE);
    for (int i = 0; i < strlen(str); ++i) {
        if (str[i] == '(') {++level; continue;}
        else if (str[i] == ')') {--level; continue;}
        else if (str[i] == ' ' && level == 0) {
            free(temp);
            temp = (char*)malloc(TEMPSIZE);
            if (check_mem() == 1) error_flag = 1; //Проверка выделений памяти
            memset(temp, '\0', TEMPSIZE);
            top = 0;
            continue;
        }
        if (level > 0) continue;
        temp[top++] = str[i]; //Добавляем еще один символ
        //temp[top] = '\0';
        //Запоминаем операции и изменяем приоритет
        //######################################################
        if (strcmp(temp, "or") == 0) {
            if (TEMPSIZE == 1){const_error = 1; error_flag = 1;} // TEMPSIZE == 1 <=> strlen(str) == 0 (TEMPSIZE больше на единицу)
            if (1 < op) op = 1;
            if (op == 1) {index = i - 1; strcpy(name_op, temp);}
        }
        else if(strcmp(temp, "xor") == 0) {
            if (TEMPSIZE == 1){const_error = 1; error_flag = 1;} // TEMPSIZE == 1 <=> strlen(str) == 0 (TEMPSIZE больше на единицу)
            if (2 < op) op = 2;
            if (op == 2) {index = i - 2; strcpy(name_op, temp);}
        }
        else if(strcmp(temp, "and") == 0) {
            if (TEMPSIZE == 1){const_error = 1; error_flag = 1;} // TEMPSIZE == 1 <=> strlen(str) == 0 (TEMPSIZE больше на единицу)
            if (3 < op) op = 3;
            if (op == 3) {index = i - 2; strcpy(name_op, temp);}
        }
        else if(strcmp(temp, "not") == 0) {
            if (TEMPSIZE == 1){const_error = 1; error_flag = 1;} // TEMPSIZE == 1 <=> strlen(str) == 0 (TEMPSIZE больше на единицу)
            if (5 < op) op = 5;
            if (op == 5 && !not_flag) {index = i - 2; strcpy(name_op, temp); not_flag = 1;}
        }
        //#################################################################################
    }
    free(temp); //Очистка памяти обязательно (хотя бы один раз она обязательно выделится)
    if (op == 10) {
        /* Если мы не нашли ни один знак на нулевом уровне, тогда пытаемся изъять значение из переменной
         * или найти ее значение в словаре
         */
        free(name_op);
        if (strcmp(str, "True") == 0) { free(str); return 1;}
        else if (strcmp(str, "False") == 0) {free(str); return 0;}
        return getValue(str);
    }
    //Правую часть выражения от найденного оператора мы в любом случае находим
    char *right; //Количество символов в right зависит от размера имени той или иной операции
    if (strcmp(name_op, "not") == 0) {
        right = strinsert(str, index+4, strlen(str)-(index+4));
        free(str);
        free(name_op);
        return !calculate(right);
    }
    //Операция "not" не нуждается в левом операнде
    //Левую часть вычисляем для всего, кроме "not" (унарная операция)
    char *left = strinsert(str, 0, index-1);
    char a, b; //Переменные для двух частей ответа
    a = calculate(left);
    if (strcmp(name_op, "or") == 0) {
        right = strinsert(str, index+3, strlen(str)-(index+3));
        free(str);
        free(name_op);
        b = calculate(right);
        //Удалять left и right нельзя! (сами удаляются внутри вызова функции calculate(exp);
        if (error_flag == 1) return 0; //В случае ошибки досрочно прерываем выполнение вычислений
        return a || b;
    }
    else if (strcmp(name_op, "and") == 0) {
        right = strinsert(str, index+4, strlen(str)-(index+4));
        free(str);
        free(name_op);
        b = calculate(right);
        return a && b;
    }
    else if (strcmp(name_op, "xor") == 0) {
        right = strinsert(str, index+4, strlen(str)-(index+4));
        free(str);
        free(name_op);
        b = calculate(right);
        return a ^ b;
    }
    //Прочие случаи
    free(str);
    free(name_op);
    return 0;
}

char check_mem() { //Функция для проверки корректного выделения памяти
    char answer = 0;
    if (errno == ENOMEM) {
        answer = 1;
        //Очищаем память под variables и все ключи
        for (int i = 0; i < top_map; ++i)
            free(variables[i].key);
        free(variables);
    }
    return answer;
}

int main(int argc, const char * argv[]) {
        errno = 0;
        variables = (Variable*)malloc(MASSIZE * sizeof(Variable)); //Словарь переменных
        if (errno == ENOMEM) {
            printf("%s", "[error]");
            return 0;
        }
        char *buffer = (char*)malloc(BUFSIZE*sizeof(char));
        if (check_mem() == 1) return 0; //Проверка выделений памяти
        memset(buffer, '\0', BUFSIZE);
        char c;
        int sem;
        int symbol_count = 0;
        while (1) {
            c = getchar();
            if (c == '\n') {
                char *tmp = remove_spaces(buffer); //Удаляем пробелы из записи присвоения переменной
                free(buffer);
                buffer = tmp;
                if ((sem = strfind(buffer, ';')) != -1) {
                    int pos = strfind(buffer, '=');
                    char *varname = strinsert(buffer, 0, pos);
                    char correct = 1; //Проверка на корректность ввода (только буквы)
                    for (int i = 0; i < strlen(varname); ++i)
                        if (!isalpha(varname[i])) {correct = 0; break;}
                    char *value_text = strinsert(buffer, pos+1, 4);
                    char value;
                    if (strcmp(value_text, "True") == 0)
                        value = 1;
                    else
                        value = 0;
                    if (strcmp(varname, "and") == 0 || strcmp(varname, "or") == 0 || //Проверка на корректность ввода имени переменной
                        strcmp(varname, "xor") == 0 || strcmp(varname, "not") == 0 ||
                        strcmp(varname, "True") == 0 || strcmp(varname, "False") == 0 ||
                        correct == 0)
                        {
                            const_error = 1;
                            error_flag = 1;
                            free(varname);
                        }
                    else
                        insert_map(varname, value);
                    free(value_text);
                    free(buffer);
                    buffer = (char*)malloc(BUFSIZE*sizeof(char));
                    if (check_mem() == 1) return 0; //Проверка выделений памяти
                    memset(buffer, '\0', BUFSIZE);
                }
                symbol_count = 0;
                continue;
            }
            else if (c == EOF) {
                if (strlen(buffer) == 0) {free(buffer); printf("%s", "False"); break;}
                char result = calculate(buffer);
                if (error_flag == 1) {printf("%s", "[error]"); break;} //Вывод [error], если поднят флаг ошибки
                if (result == 1)
                    printf("%s", "True");
                else
                    printf("%s", "False");
                break;
            }
            if (symbol_count+1 >= BUFSIZE) {
                buffer = (char*)realloc(buffer, BUFSIZE *= 2);
                if (check_mem() == 1) return 0; //Проверка выделений памяти
            }
            buffer[symbol_count++] = c;
            buffer[symbol_count] = '\0';
        }
    //Очищаем память под variables и все ключи
    for (int i = 0; i < top_map; ++i)
       free(variables[i].key);
    free(variables);
    return 0;
}

