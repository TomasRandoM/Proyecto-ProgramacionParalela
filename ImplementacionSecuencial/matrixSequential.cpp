#include <iostream>
#include <chrono>

using namespace std;

void printMatrix(float** matrix, int n, int m);

int main() {
    //n y m son las filas y columnas de la matriz 1. n1 y m1 son las filas y columnas de la matriz 2.
    int n, m, n1, m1;
    /*
    n = 3;
    m = 3;
    n1 = 3;
    m1 = 3;
    */

    //Verifico que la multipicación se pueda hacer
    if (m != n1) {
        cout << "ERROR. Las matrices deben ser de tamaño n x m y m x k respectivamente." << endl;
        return 0;
    }

    auto start = chrono::high_resolution_clock::now();

    //Declaración de matrices (matriz 1, 2 y donde se almacenará el resultado)
    float** matrix = new float*[n];
    float** matrix2 = new float*[n1];
    float** matrixRes = new float*[n];

    for (int i = 0; i < n; i++) {
        matrix[i] = new float[m];
        matrixRes[i] = new float[m1];
    }
    for (int i = 0; i < n1; i++) {
        matrix2[i] = new float[m1];
    }

    /*
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            matrix[i][j] = 2;
            matrix2[i][j] = j;
        }
    }
    */

    //Proceso de multiplicación de matrices
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            matrixRes[i][j] = 0;
            for (int k = 0; k < m; k++) {
                matrixRes[i][j] += matrix[i][k] * matrix2[k][j];
            }
        }
    }

    /*
    cout << "Matriz 1:" << endl;
    printMatrix(matrix, n, m);
    cout << "Matriz 2:" << endl;
    printMatrix(matrix2, n1, m1);
    cout << "Matriz resultado:" << endl;
    printMatrix(matrixRes, n, m1);
    */

    //Se libera la memoria utilizada
    for (int i = 0; i < n; i++) {
        delete[] matrix[i];
        delete[] matrixRes[i];
    }
    for (int i = 0; i < n1; i++) {
        delete[] matrix2[i];
    }

    delete[] matrix;
    delete[] matrix2;
    delete[] matrixRes;
    auto end = chrono::high_resolution_clock::now();
    chrono::duration<double> duration = end - start;
    cout << "El tiempo de ejecución fue de: " << duration.count() << endl;
}

//Imprime la matriz, recibe como parámetros la matriz y la cantidad de filas y columnas
void printMatrix(float** matrix, int n, int m) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            cout << matrix[i][j] << " ";
        }
        cout << endl;
    }
}