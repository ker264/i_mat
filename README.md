# i_mat

Library for reading mat files into gsl structures and standard types of C data

## Dependencies
* zlip - mingw64/mingw-w64-x86_64-zlib 1.2.13-3 
Compression library implementing the deflate compression method found in gzip and PKZIP (mingw-w64)
* gsl - GNU Sciencific Library 2.7

## Requirements for mat file
* One parameter per one mat file
* Works with matrices of size AxB

## Types of data that the library works with
<table>
    <tr>
        <th>gsl type</th>
        <th>matlab type</th>
    </tr>
    <tr>
        <td>double</td>
        <td>double</td>
    </tr>
    <tr>
        <td>int</td>
        <td>int</td>
    </tr>
    <tr>
        <td>complex double</td>
        <td>complex double</td>
    </tr>
    <tr>
        <td>gsl_vector</td>
        <td>double vector</td>
    </tr>
    <tr>
        <td>gsl_vector_int</td>
        <td>int vector</td>
    </tr>
    <tr>
        <td>gsl_vector_complex</td>
        <td>complex double vector</td>
    </tr>
    <tr>
        <td>gsl_matrix</td>
        <td>double matrix</td>
    </tr>
    <tr>
        <td>gsl_matrix_int</td>
        <td>int matrix</td>
    </tr>
    <tr>
        <td>gsl_matrix_complex</td>
        <td>complex double matrix</td>
    </tr>
</table>

### Library does not work with cell structures

### Example of work
```
#include "i_mat.h"

int main(int argc, char const *argv[])
{
    IMatError mError;

    // vector
    gsl_vector *S_test = openMatVector("S.mat", &mError);
    if (mError.isErr)
    {
        printf("\nS_test: %s", mError.stringErr);
    }
    else
    {
        printf("\nS_test: %d элементов", S_test->size);
    }

    // int
    int a = openMatInt("k.mat", &mError);
    if (mError.isErr)
    {
        printf("\na: %s", mError.stringErr);
    }
    else
    {
        printf("\na: %d", a);
    }
}
```
