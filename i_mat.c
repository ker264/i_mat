#include <complex.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <zlib.h>

// #include <libdeflate.h>

#include <gsl/gsl_matrix.h>
#include <gsl/gsl_vector.h>

enum EDataTypes
{
    DT_INT,
    DT_INT_64,
    DT_DOUBLE,
    DT_COMPLEX,
    DT_VECTOR,
    DT_VECTOR_INT,
    DT_VECTOR_COMPLEX,
    DT_MATRIX,
    DT_MATRIX_INT,
    DT_MATRIX_COMPLEX,
    DT_EMPTY
};

static const char *dataTypeNames[] = {"INT",    "INT_64",     "DOUBLE",         "COMPLEX", "VECTOR", "VECTOR_INT", "VECTOR_COMPLEX",
                                      "MATRIX", "MATRIX_INT", "MATRIX_COMPLEX", "EMPTY",   "UNKNOWN"};

typedef struct
{
    int isErr;
    char stringErr[264];
} IMatError;

typedef struct
{
    char name[40];
    enum EDataTypes type;
    int sizeI;
    int sizeJ;
    int64_t *dataInt;
    double *dataDouble;
    unsigned char *zipData;
} ElmementInfo;

void cleanError(IMatError *error);
void setError(IMatError *error, const char *format, ...);

void openMatEngine(char *filePath, ElmementInfo *eInfo, IMatError *error);

void handleMatHeader(FILE *file, IMatError *error);
void handleElement(FILE *file, IMatError *error, ElmementInfo *eInfo);
void handleIntSequence(int64_t *destinationArray, unsigned char *intDataInByteSeq, int byteInInt, int size, int isSigned);
void handleDoubleSequence(double *destinationArray, unsigned char *doubleDataInByteSeq, int size);
void uncompressElement(IMatError *error, ElmementInfo *eInfo);
int decompressData(const unsigned char *compressed_data, int compressed_size, unsigned char **uncompressed_data, int *uncompressed_size);

int64_t readFromByteInt64(unsigned char *byteSeq);
int readFromByteInt32(unsigned char *byteSeq);
int readFromByteInt16(unsigned char *byteSeq);
int readFromByteInt8(unsigned char *byteSeq);

uint64_t readFromByteUInt64(unsigned char *byteSeq);
int readFromByteUInt32(unsigned char *byteSeq);
int readFromByteUInt16(unsigned char *byteSeq);
int readFromByteUInt8(unsigned char *byteSeq);

int64_t readFromByteWithOffset(unsigned char *byteSeq, int byteInInt, int isSigned);
int calculateByteSizeFromMatDataCode(int dataType);
enum EDataTypes decideType(int sizeI, int sizeJ, int isComplex, int typeCode);
char *getDataTypeName(enum EDataTypes dataType);

int openMatInt(char *filePath, IMatError *error)
{
    cleanError(error);

    ElmementInfo eInfo;

    openMatEngine(filePath, &eInfo, error);
    if (error->isErr)
        return 0;

    if (eInfo.type != DT_INT)
    {
        setError(error, "Got error while reading %s. Expected %s, but readed %s", eInfo.name, getDataTypeName(DT_INT),
                 getDataTypeName(eInfo.type));
        return 0;
    }

    return eInfo.dataInt[0];
}

int64_t openMatInt64(char *filePath, IMatError *error)
{
    cleanError(error);

    ElmementInfo eInfo;

    openMatEngine(filePath, &eInfo, error);
    if (error->isErr)
        return 0;

    if (eInfo.type != DT_INT_64)
    {
        setError(error, "Got error while reading %s. Expected %s, but readed %s", eInfo.name, getDataTypeName(DT_INT_64),
                 getDataTypeName(eInfo.type));
        return 0;
    }

    return eInfo.dataInt[0];
}

double openMatDouble(char *filePath, IMatError *error)
{
    cleanError(error);

    ElmementInfo eInfo;

    openMatEngine(filePath, &eInfo, error);
    if (error->isErr)
        return 0;

    // TODO убрать dtint
    if (eInfo.type != DT_DOUBLE && eInfo.type != DT_INT)
    {
        setError(error, "Got error while reading %s. Expected %s, but readed %s", eInfo.name, getDataTypeName(DT_DOUBLE),
                 getDataTypeName(eInfo.type));
        return 0;
    }

    return eInfo.type == DT_INT ? (double)eInfo.dataInt[0] : eInfo.dataDouble[0];
}

gsl_vector *openMatVector(char *filePath, IMatError *error)
{
    gsl_vector *result;
    cleanError(error);

    ElmementInfo eInfo;

    openMatEngine(filePath, &eInfo, error);
    if (error->isErr)
        return result;

    int elemsNumber = eInfo.sizeI * eInfo.sizeJ;

    if (eInfo.type != DT_VECTOR && eInfo.type != DT_DOUBLE && eInfo.type != DT_EMPTY)
    {
        if (eInfo.type == DT_VECTOR_INT || eInfo.type == DT_INT)
        {
            result = gsl_vector_alloc(elemsNumber);

            for (int i = 0; i < elemsNumber; i++)
            {
                gsl_vector_set(result, i, eInfo.dataInt[i]);
            }
            free(eInfo.dataInt);
            return result;
        }
        setError(error, "Got error while reading %s. Expected %s, but readed %s", eInfo.name, getDataTypeName(DT_VECTOR),
                 getDataTypeName(eInfo.type));
        return result;
    }

    result = gsl_vector_alloc(elemsNumber);

    for (int i = 0; i < elemsNumber; i++)
    {
        gsl_vector_set(result, i, eInfo.dataDouble[i]);
    }

    if (eInfo.type != DT_EMPTY)
        free(eInfo.dataDouble);
    return result;
}

gsl_vector_int *openMatVectorInt(char *filePath, IMatError *error)
{
    gsl_vector_int *result;
    cleanError(error);

    ElmementInfo eInfo;

    openMatEngine(filePath, &eInfo, error);
    if (error->isErr)
        return result;

    if (eInfo.type != DT_VECTOR_INT && eInfo.type != DT_INT && eInfo.type != DT_EMPTY)
    {
        setError(error, "Got error while reading %s. Expected %s, but readed %s", eInfo.name, getDataTypeName(DT_VECTOR_INT),
                 getDataTypeName(eInfo.type));
        return result;
    }

    int elemsNumber = eInfo.sizeI * eInfo.sizeJ;

    result = gsl_vector_int_alloc(elemsNumber);

    for (int i = 0; i < elemsNumber; i++)
    {
        gsl_vector_int_set(result, i, eInfo.dataInt[i]);
    }

    if (eInfo.type != DT_EMPTY)
        free(eInfo.dataInt);
    return result;
}

gsl_matrix *openMatMatrix(char *filePath, IMatError *error)
{
    gsl_matrix *result;
    cleanError(error);

    ElmementInfo eInfo;

    openMatEngine(filePath, &eInfo, error);
    if (error->isErr)
        return result;

    if (eInfo.type != DT_MATRIX && eInfo.type != DT_VECTOR && eInfo.type != DT_DOUBLE && eInfo.type != DT_EMPTY)
    {
        if (eInfo.type == DT_MATRIX_INT || eInfo.type == DT_VECTOR_INT || eInfo.type == DT_INT)
        {
            result = gsl_matrix_alloc(eInfo.sizeI, eInfo.sizeJ);

            for (int i = 0; i < eInfo.sizeI; i++)
            {
                for (int j = 0; j < eInfo.sizeJ; j++)
                {
                    gsl_matrix_set(result, i, j, eInfo.dataInt[i + eInfo.sizeI * j]);
                }
            }

            free(eInfo.dataInt);
            return result;
        }

        setError(error, "Got error while reading %s. Expected %s, but readed %s", eInfo.name, getDataTypeName(DT_MATRIX),
                 getDataTypeName(eInfo.type));
        return result;
    }

    result = gsl_matrix_alloc(eInfo.sizeI, eInfo.sizeJ);

    for (int i = 0; i < eInfo.sizeI; i++)
    {
        for (int j = 0; j < eInfo.sizeJ; j++)
        {
            gsl_matrix_set(result, i, j, eInfo.dataDouble[i + eInfo.sizeI * j]);
        }
    }

    if (eInfo.type != DT_EMPTY)
        free(eInfo.dataDouble);
    return result;
}

gsl_matrix_int *openMatMatrixInt(char *filePath, IMatError *error)
{
    gsl_matrix_int *result;
    cleanError(error);

    ElmementInfo eInfo;

    openMatEngine(filePath, &eInfo, error);
    if (error->isErr)
        return result;

    if (eInfo.type != DT_MATRIX_INT && eInfo.type != DT_VECTOR_INT && eInfo.type != DT_INT && eInfo.type != DT_EMPTY)
    {
        setError(error, "Got error while reading %s. Expected %s, but readed %s", eInfo.name, getDataTypeName(DT_MATRIX_INT),
                 getDataTypeName(eInfo.type));
        return result;
    }

    result = gsl_matrix_int_alloc(eInfo.sizeI, eInfo.sizeJ);

    for (int i = 0; i < eInfo.sizeI; i++)
    {
        for (int j = 0; j < eInfo.sizeJ; j++)
        {
            gsl_matrix_int_set(result, i, j, eInfo.dataInt[i + eInfo.sizeI * j]);
        }
    }

    if (eInfo.type != DT_EMPTY)
        free(eInfo.dataInt);
    return result;
}

complex double openMatComplex(char *filePath, IMatError *error)
{
    complex double result;
    cleanError(error);

    ElmementInfo eInfo;

    openMatEngine(filePath, &eInfo, error);
    if (error->isErr)
        return result;

    if (eInfo.type != DT_COMPLEX)
    {
        setError(error, "Got error while reading %s. Expected %s, but readed %s", eInfo.name, getDataTypeName(DT_COMPLEX),
                 getDataTypeName(eInfo.type));
        return result;
    }

    result = eInfo.dataDouble[0] + I * eInfo.dataDouble[1];

    free(eInfo.dataDouble);
    return result;
}

gsl_vector_complex *openMatVectorComplex(char *filePath, IMatError *error)
{
    gsl_vector_complex *result;
    cleanError(error);

    ElmementInfo eInfo;

    openMatEngine(filePath, &eInfo, error);
    if (error->isErr)
        return result;

    if (eInfo.type != DT_VECTOR_COMPLEX && eInfo.type != DT_COMPLEX && eInfo.type != DT_EMPTY)
    {
        setError(error, "Got error while reading %s. Expected %s, but readed %s", eInfo.name, getDataTypeName(DT_VECTOR_COMPLEX),
                 getDataTypeName(eInfo.type));
        return result;
    }

    int elemsNumber = eInfo.sizeI * eInfo.sizeJ;

    result = gsl_vector_complex_alloc(elemsNumber);

    for (int i = 0; i < elemsNumber; i++)
    {
        gsl_vector_complex_set(result, i, eInfo.dataDouble[i] + I * eInfo.dataDouble[i + elemsNumber]);
    }

    if (eInfo.type != DT_EMPTY)
        free(eInfo.dataDouble);
    return result;
}

gsl_matrix_complex *openMatMatrixComplex(char *filePath, IMatError *error)
{
    gsl_matrix_complex *result;
    cleanError(error);

    ElmementInfo eInfo;

    openMatEngine(filePath, &eInfo, error);
    if (error->isErr)
        return result;

    if (eInfo.type != DT_MATRIX_COMPLEX && eInfo.type != DT_VECTOR_COMPLEX && eInfo.type != DT_COMPLEX && eInfo.type != DT_EMPTY)
    {
        setError(error, "Got error while reading %s. Expected %s, but readed %s", eInfo.name, getDataTypeName(DT_MATRIX_COMPLEX),
                 getDataTypeName(eInfo.type));
        return result;
    }

    result = gsl_matrix_complex_alloc(eInfo.sizeI, eInfo.sizeJ);
    int elemsNumber = eInfo.sizeI * eInfo.sizeJ;

    for (int i = 0; i < eInfo.sizeI; i++)
    {
        for (int j = 0; j < eInfo.sizeJ; j++)
        {
            gsl_matrix_complex_set(result, i, j,
                                   eInfo.dataDouble[i + eInfo.sizeI * j] + I * eInfo.dataDouble[elemsNumber + i + eInfo.sizeI * j]);
        }
    }

    if (eInfo.type != DT_EMPTY)
        free(eInfo.dataDouble);
    return result;
}

void openMatEngine(char *filePath, ElmementInfo *eInfo, IMatError *error)
{
    FILE *file = fopen(filePath, "rb");
    if (file == NULL)
    {
        char errString[264];
        sprintf(errString, "Unable to open file: %s", filePath);
        setError(error, errString);
        return;
    }

    handleMatHeader(file, error);
    if (error->isErr)
    {
        fclose(file);
        return;
    }

    handleElement(file, error, eInfo);

    fclose(file);
    return;
}

/**
 * Очищает ошибку
 */
void cleanError(IMatError *error)
{
    error->isErr = 0;
    sprintf(error->stringErr, "");
}

/**
 * Задает ошибке переданную строку
 *
 * @param error ошибка, которую нужно установить
 * @param format форматированная строка с описанием ошибки
 * @param ... список аргументов для форматированной строки
 */
void setError(IMatError *error, const char *format, ...)
{
    va_list args;

    va_start(args, format);
    vsprintf(error->stringErr, format, args);
    va_end(args);

    error->isErr = 1;
}

/**
 * Проверяем header mat-файла
 */
void handleMatHeader(FILE *file, IMatError *error)
{
    unsigned char *header = (unsigned char *)malloc(128);

    fread(header, 128, 1, file);

    char description[117];
    strncpy(description, header, 116);
    description[117] = '\0';

    if (header[126] != 'I')
    {
        setError(error, "Unexpected Byte Order");
        free(header);
        return;
    }

    free(header);
}

/**
 * Обработка заголовочного тега элемента
 */
void handleElement(FILE *file, IMatError *error, ElmementInfo *eInfo)
{
    // Читаем мета данные скомпресированных данных и извлекаем их
    unsigned char *tagField = (unsigned char *)malloc(8);
    fread(tagField, 8, 1, file);

    eInfo->sizeI = readFromByteInt32(&(tagField[4]));
    eInfo->zipData = (unsigned char *)malloc(eInfo->sizeI);

    fread(eInfo->zipData, eInfo->sizeI, 1, file);

    free(tagField);

    uncompressElement(error, eInfo);
    if (error->isErr)
        return;

    if (eInfo->zipData[0] != 14)
    {
        setError(error, "Unexpected format of ungzipted data");
        return;
    }

    // Проверка на комплексное значение
    // TODO добавить определение типа массива
    unsigned char complexFlagMask = 1 << 3;
    int isComplex = 0;
    unsigned char *complexDataPointer;

    if (complexFlagMask & eInfo->zipData[17])
        isComplex = 1;

    // Получаем размеры элемента
    // TODO добавить поддержку многомерных массивов
    if (eInfo->zipData[28] != 8)
    {
        setError(error, "Multidimensional data");
        return;
    }

    eInfo->sizeI = readFromByteInt32(&(eInfo->zipData[32]));
    eInfo->sizeJ = readFromByteInt32(&(eInfo->zipData[36]));
    int expectedSize = eInfo->sizeI * eInfo->sizeJ;

    // Если записан пустой элемент
    if (expectedSize == 0)
    {
        eInfo->type = DT_EMPTY;
        return;
    }

    // Получаем имя и начало данных
    int nameLength = eInfo->zipData[42] ? eInfo->zipData[42] : readFromByteInt32(&eInfo->zipData[44]);
    int dataStartIndex = eInfo->zipData[42] ? 48 : 40 + 8 + ceil(nameLength / 8.0) * 8;
    memcpy(eInfo->name, &eInfo->zipData[eInfo->zipData[42] ? 44 : 48], nameLength);
    eInfo->name[nameLength] = '\0';

    int dataTypeCode = eInfo->zipData[dataStartIndex];

    eInfo->type = decideType(eInfo->sizeI, eInfo->sizeJ, isComplex, dataTypeCode);
    //  Определяем signed или unsigned int
    int isSigned = (dataTypeCode == 1 || dataTypeCode == 3 || dataTypeCode == 5 || dataTypeCode == 12) ? 1 : 0;

    // TODO Вынести handle Data в отдельную функцию, результа которой обрабатывать в зависимости от complex
    // Определяем элемент int или double
    if (dataTypeCode < 7 || dataTypeCode == 12 || dataTypeCode == 13)
    {
        // Проверяем smalldata или нет
        int isSmallData = eInfo->zipData[dataStartIndex + 2];

        int byteInInt = calculateByteSizeFromMatDataCode(dataTypeCode);

        // Меняем тип на int_64 если надо
        if (byteInInt == 8)
            eInfo->type = DT_INT_64;

        // Проверка на совпадение размеров
        int realSize = (isSmallData ? readFromByteInt16(&(eInfo->zipData[dataStartIndex + 2]))
                                    : readFromByteInt32(&(eInfo->zipData[dataStartIndex + 4]))) /
                       byteInInt;

        if (realSize != expectedSize)
        {
            setError(error, "Expected and real data sizes don't match");
            return;
        }

        // Создаем буферный массив для дальнейшей проверки является ли это комплексными данными или нет
        int64_t *destinationArray = (int64_t *)malloc(sizeof(int64_t) * expectedSize);

        // Чуть меняем указатели в зависимости от small data или нет
        handleIntSequence(destinationArray, &(eInfo->zipData[dataStartIndex + (isSmallData ? 4 : 8)]), byteInInt, expectedSize, isSigned);

        if (isComplex)
        {
            // Задаем указатель для комплексной части
            int dataLengthWithTag = isSmallData ? 8 : 8 + ceil(1. * readFromByteInt32(&(eInfo->zipData[dataStartIndex + 4])) / 8) * 8;
            complexDataPointer = &(eInfo->zipData[dataStartIndex + dataLengthWithTag]);

            eInfo->dataDouble = (double *)malloc(sizeof(double) * 2 * expectedSize);

            for (int i = 0; i < expectedSize; i++)
            {
                eInfo->dataDouble[i] = (double)(destinationArray[i]);
            }

            free(destinationArray);
        }
        else
        {
            eInfo->dataInt = destinationArray;
        }
    }
    else if (dataTypeCode == 9)
    {
        eInfo->dataDouble = (double *)malloc(sizeof(double) * expectedSize * (isComplex ? 2 : 1));

        // Проверка на совпадение размеров
        if (readFromByteInt32(&(eInfo->zipData[dataStartIndex + 4])) / 8 != expectedSize)
        {
            setError(error, "Expected and real data sizes don't match");
            return;
        }

        handleDoubleSequence(eInfo->dataDouble, &(eInfo->zipData[dataStartIndex + 8]), expectedSize);

        // Задаем указатель для комплексной части (будет использован только если она существует)
        complexDataPointer = &(eInfo->zipData[dataStartIndex + 8 * (expectedSize + 1)]);
    }
    else
    {
        setError(error, "Unexpected type when reading data sequence");
        return;
    }

    // FIXME Временная часть, убрать когда будет переделана обработка элемента
    if (isComplex)
    {
        // Проверяем вдруг все воображаемые части чисел записаны в int(лол)
        dataTypeCode = complexDataPointer[0];

        if (dataTypeCode == 9)
        {
            handleDoubleSequence(&(eInfo->dataDouble[expectedSize]), &(complexDataPointer[8]), expectedSize);
        }
        else if (dataTypeCode < 7 || dataTypeCode == 12 || dataTypeCode == 13)
        {
            // Проверяем smalldata или нет
            int isSmallData = complexDataPointer[2];

            int byteInInt = calculateByteSizeFromMatDataCode(dataTypeCode);

            // Создаем буферный массив для переноса в double
            int64_t *destinationArray = (int64_t *)malloc(sizeof(int64_t) * expectedSize);

            // Чуть меняем указатели в зависимости от small data или нет
            handleIntSequence(destinationArray, &(complexDataPointer[isSmallData ? 4 : 8]), byteInInt, expectedSize, isSigned);

            for (int i = 0; i < expectedSize; i++)
            {
                eInfo->dataDouble[expectedSize + i] = (double)(destinationArray[i]);
            }

            free(destinationArray);
        }
        else
        {
            setError(error, "unexpected type when reading complex part of data sequence");
            return;
        }
    }
}

/**
 * Обработка int последовательности
 */
void handleIntSequence(int64_t *destinationArray, unsigned char *intDataInByteSeq, int byteInInt, int size, int isSigned)
{
    for (int i = 0; i < size; i++)
    {
        destinationArray[i] = readFromByteWithOffset(&(intDataInByteSeq[i * byteInInt]), byteInInt, isSigned);
    }
}

/**
 * Обработка double последовательности
 */
void handleDoubleSequence(double *destinationArray, unsigned char *doubleDataInByteSeq, int size)
{
    for (int i = 0; i < size; i++)
    {
        destinationArray[i] = ((double *)doubleDataInByteSeq)[i];
    }
}

/**
 * Разархивировать сжатые данные
 */
void uncompressElement(IMatError *error, ElmementInfo *eInfo)
{
    unsigned char *spaceToUncompress;

    int uncompressedSize = eInfo->sizeI < 214748364 ? eInfo->sizeI * 10 : 2147483647;

    int result = decompressData(eInfo->zipData, eInfo->sizeI, &spaceToUncompress, &uncompressedSize);

    if (result != Z_OK)
    {
        char errStr[80];
        sprintf(errStr, "Failed to uncompress element, err: %d", result);
        setError(error, errStr);
        free(spaceToUncompress);

        return;
    }

    // Заменяем сжатый элемент на разжатый
    free(eInfo->zipData);
    eInfo->zipData = (unsigned char *)malloc(uncompressedSize);
    memcpy(eInfo->zipData, spaceToUncompress, uncompressedSize);
    free(spaceToUncompress);
}

/**
 * Разархивировать сжатые данные по частям
 *
 * Входные параметры:
 *  compressed_data: const unsigned char* - сжатые данные
 *  compressed_size: int                  - размер сжатых данных
 *  uncompressed_data: unsigned char**    - указатель на выходной буфер
 *  uncompressed_size: int*               - указатель на размер выходного буфера
 *
 * Возвращаемое значение:
 * ret: int - 0 - успех,не 0 - ошибка
 */
int decompressData(const unsigned char *compressed_data, int compressed_size, unsigned char **uncompressed_data, int *uncompressed_size)
{
    // Размер буфера
    const int CHUNK = *uncompressed_size;

    // Очищаем выходный буфер
    *uncompressed_size = 0;
    *uncompressed_data = NULL;

    int ret;
    unsigned have;
    z_stream strm;
    unsigned char *out = (unsigned char *)malloc(CHUNK);
    if (out == NULL)
    {
        return Z_MEM_ERROR;
    }

    // Initialize zlib stream
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.avail_in = 0;
    strm.next_in = Z_NULL;
    ret = inflateInit(&strm);
    if (ret != Z_OK)
    {
        return ret;
    }

    strm.avail_in = compressed_size;
    strm.next_in = (unsigned char *)compressed_data;

    // Decompress data
    do
    {
        strm.avail_out = CHUNK;
        strm.next_out = out;
        ret = inflate(&strm, Z_SYNC_FLUSH);
        have = CHUNK - strm.avail_out;

        // Reallocate memory for uncompressed data
        *uncompressed_data = (unsigned char *)realloc(*uncompressed_data, *uncompressed_size + have);
        if (*uncompressed_data == NULL)
        {
            return Z_MEM_ERROR;
        }

        // Copy decompressed data to output buffer
        memcpy(*uncompressed_data + *uncompressed_size, out, have);
        *uncompressed_size += have;
    } while (ret == Z_OK);

    // Clean up
    inflateEnd(&strm);

    if (ret == Z_STREAM_END || ret == Z_BUF_ERROR)
    {
        return Z_OK;
    }

    return ret;
}

// Чтение знаковых int
int64_t readFromByteInt64(unsigned char *byteSeq)
{
    int64_t num = 0;
    memcpy(&num, byteSeq, sizeof(int64_t));
    return num;
}

int readFromByteInt32(unsigned char *byteSeq)
{
    return (int)(((char)byteSeq[3] << 24) | (byteSeq[2] << 16) | (byteSeq[1] << 8) | byteSeq[0]);
}

int readFromByteInt16(unsigned char *byteSeq)
{
    return (int)(((char)byteSeq[1] << 8) | byteSeq[0]);
}

int readFromByteInt8(unsigned char *byteSeq)
{
    return (int)((char)byteSeq[0]);
}

// Чтение без знаковых uint
uint64_t readFromByteUInt64(unsigned char *byteSeq)
{
    uint64_t num = 0;
    memcpy(&num, byteSeq, sizeof(uint64_t));
    return num;
}

int readFromByteUInt32(unsigned char *byteSeq)
{
    return (int)((byteSeq[3] << 24) | (byteSeq[2] << 16) | (byteSeq[1] << 8) | byteSeq[0]);
}

int readFromByteUInt16(unsigned char *byteSeq)
{
    return (int)((byteSeq[1] << 8) | byteSeq[0]);
}

int readFromByteUInt8(unsigned char *byteSeq)
{
    return (int)(byteSeq[0]);
}

/**
 * Читаем из байтовой последовательности
 */
int64_t readFromByteWithOffset(unsigned char *byteSeq, int byteInInt, int isSigned)
{
    switch (byteInInt)
    {
    case 1:
        return isSigned ? readFromByteInt8(byteSeq) : readFromByteUInt8(byteSeq);

    case 2:
        return isSigned ? readFromByteInt16(byteSeq) : readFromByteUInt16(byteSeq);

    case 4:
        return isSigned ? readFromByteInt32(byteSeq) : readFromByteUInt32(byteSeq);

    case 8:
        return isSigned ? readFromByteInt64(byteSeq) : readFromByteUInt64(byteSeq);
    }
}

/**
 * @brief Получаем количество байт на хранение одного int числа в зависимости от dataType
 */
int calculateByteSizeFromMatDataCode(int dataType)
{
    switch (dataType)
    {
    case 1:
    case 2:
        return 1;
    case 3:
    case 4:
        return 2;
    case 5:
    case 6:
        return 4;
    case 12:
    case 13:
        return 8;
    }
}

/**
 * @brief Определяет тип данных в зависимости от размеров матрицы и кода типа данных
 *
 * @param sizeI Количество строк
 * @param sizeJ Количество столбцов
 * @param isComplex Является ли матрица комплексной
 * @param typeCode Код типа данных
 * @return Тип данных
 */
enum EDataTypes decideType(int sizeI, int sizeJ, int isComplex, int typeCode)
{
    enum EDataTypes dataType;

    if (sizeI == 1)
    {
        if (sizeJ == 1)
        {
            dataType = isComplex ? DT_COMPLEX : (typeCode != 9 ? DT_INT : DT_DOUBLE);
        }
        else
        {
            dataType = isComplex ? DT_VECTOR_COMPLEX : (typeCode != 9 ? DT_VECTOR_INT : DT_VECTOR);
        }
    }
    else
    {
        if (sizeJ == 1)
        {
            dataType = isComplex ? DT_VECTOR_COMPLEX : (typeCode != 9 ? DT_VECTOR_INT : DT_VECTOR);
        }
        else
        {
            dataType = isComplex ? DT_MATRIX_COMPLEX : (typeCode != 9 ? DT_MATRIX_INT : DT_MATRIX);
        }
    }

    return dataType;
}

/**
 * @brief Get name of data type
 *
 * @param dataType Data type
 * @return const char* Name of data type
 */
char *getDataTypeName(enum EDataTypes dataType)
{
    return (char *)dataTypeNames[dataType];
}
