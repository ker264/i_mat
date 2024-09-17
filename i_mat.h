#ifndef I_MAT_H_
#define I_MAT_H_

#include <complex.h>
#include <stdint.h>

#include <gsl/gsl_matrix.h>
#include <gsl/gsl_vector.h>

/**
 * Структура для отслеживания ошибок при выполнени функции
 *
 * isErr: int - если 1, то есть ошибки, если 0 - нет
 * stringErr: char* - если есть ошибки, то тут текст этой ошибки
 */
typedef struct
{
    int isErr;
    char stringErr[264];
} IMatError;

/**
 * double res = openMatDouble("file.mat", error);
 *
 * Чтение единичного double значения из mat файла
 *
 * Входные данные:
 *  filePath: char* - путь к mat файлу
 *  error: IMatError - структура для отслеживания ошибок
 *
 * Возвращаемый параметр:
 *  res: double - результат выполнения функции
 */
double openMatDouble(char *filePath, IMatError *error);

/**
 * int64_t res = openMatInt64("file.mat", error);
 *
 * Чтение единичного int значения из mat файла
 *
 * Входные данные:
 *  filePath: char* - путь к mat файлу
 *  error: IMatError - структура для отслеживания ошибок
 *
 * Возвращаемый параметр:
 *  res: int - результат выполнения функции
 */
int64_t openMatInt64(char *filePath, IMatError *error);

/**
 * int res = openMatInt("file.mat", error);
 *
 * Чтение единичного int значения из mat файла
 *
 * Входные данные:
 *  filePath: char* - путь к mat файлу
 *  error: IMatError - структура для отслеживания ошибок
 *
 * Возвращаемый параметр:
 *  res: int - результат выполнения функции
 */
int openMatInt(char *filePath, IMatError *error);

/**
 * complex double res = openMatComplex("file.mat", error);
 *
 * Чтение единичного double значения из mat файла
 *
 * Входные данные:
 *  filePath: char* - путь к mat файлу
 *  error: IMatError - структура для отслеживания ошибок
 *
 * Возвращаемый параметр:
 *  res: complex double - результат выполнения функции
 */
complex double openMatComplex(char *filePath, IMatError *error);

/**
 * gsl_vector *res = openMatVector("file.mat", error);
 *
 * Чтение vector double (одномерный массив double) из mat файла
 *
 * Входные данные:
 *  filePath: char* - путь к mat файлу
 *  error: IMatError - структура для отслеживания ошибок
 *
 * Возвращаемый параметр:
 *  res: gsl_vector* - результат выполнения функции
 */
gsl_vector *openMatVector(char *filePath, IMatError *error);

/**
 * gsl_vector_int *res = openMatVectorInt("file.mat", error);
 *
 * Чтение vector int (одномерный массив int) из mat файла
 *
 * Входные данные:
 *  filePath: char* - путь к mat файлу
 *  error: IMatError - структура для отслеживания ошибок
 *
 * Возвращаемый параметр:
 *  res: gsl_vector_int* - результат выполнения функции
 */
gsl_vector_int *openMatVectorInt(char *filePath, IMatError *error);

/**
 * gsl_vector_complex *res = openMatVectorComplex("file.mat", error);
 *
 * Чтение vector complex double (одномерный массив complex double) из mat файла
 *
 * Входные данные:
 *  filePath: char* - путь к mat файлу
 *  error: IMatError - структура для отслеживания ошибок
 *
 * Возвращаемый параметр:
 *  res: gsl_vector_complex* - результат выполнения функции
 */
gsl_vector_complex *openMatVectorComplex(char *filePath, IMatError *error);

/**
 * gsl_matrix *res = openMatMatrix("file.mat", error);
 *
 * Чтение matrix double (двумерный массив double) из mat файла
 *
 * Входные данные:
 *  filePath: char* - путь к mat файлу
 *  error: IMatError - структура для отслеживания ошибок
 *
 * Возвращаемый параметр:
 *  res: gsl_matrix* - результат выполнения функции
 */
gsl_matrix *openMatMatrix(char *filePath, IMatError *error);

/**
 * gsl_matrix_int *res = openMatMatrix("file.mat", error);
 *
 * Чтение matrix int (двумерный массив int) из mat файла
 *
 * Входные данные:
 *  filePath: char* - путь к mat файлу
 *  error: IMatError - структура для отслеживания ошибок
 *
 * Возвращаемый параметр:
 *  res: gsl_matrix_int* - результат выполнения функции
 */
gsl_matrix_int *openMatMatrixInt(char *filePath, IMatError *error);

/**
 * gsl_matrix_complex *res = openMatMatrixComplex("file.mat", error);
 *
 * Чтение matrix double (двумерный массив double) из mat файла
 *
 * Входные данные:
 *  filePath: char* - путь к mat файлу
 *  error: IMatError - структура для отслеживания ошибок
 *
 * Возвращаемый параметр:
 *  res: gsl_matrix_complex* - результат выполнения функции
 */
gsl_matrix_complex *openMatMatrixComplex(char *filePath, IMatError *error);

#endif // I_MAT_H_