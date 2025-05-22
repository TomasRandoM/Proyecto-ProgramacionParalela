#include <iostream>
#include <chrono>
#include <random>
using namespace std;

void printMatrix(float** matrix, int n, int m);

int main(int argc, char** argv) {
    //n y m son las filas y columnas de la matriz 1. n1 y m1 son las filas y columnas de la matriz 2.
    int n, m, n1, m1;
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> dis(1, 300);

    if (argc == 5 ) {
        n = atoi(argv[1]);
        m = atoi(argv[2]);
        m1 = atoi(argv[4]);
        n1 = atoi(argv[3]);
    } else {
        cout << "ERROR. El número de parámetros ingresados no es correcto. Debe indicar (filasA) (columnasA) (filasB) (columnasB)" << endl;
        return 0;
    }

    //Verifico que la multipicación se pueda hacer
    if (m != n1) {
        cout << "ERROR. Las matrices deben ser de tamaño n x m y m x k respectivamente." << endl;
        return 0;
    }

    auto start = chrono::high_resolution_clock::now();

    //Declaración de matrices (matriz 1, 2 y donde se almacenará el resultado)
    float* matrixdata = new float[n * m];
    float* matrixdata2 = new float[n1 * m1];
    float* matrixdataRes = new float[n * m1];
    float** matrix = new float*[n];
    float** matrix2 = new float*[n1];
    float** matrixRes = new float*[n];

    //Inicializo las matrices con números random
    for (int i = 0; i < n * m; i++) {
        matrixdata[i] = dis(gen);
    }
    for (int i = 0; i < n1 * m1; i++) {
        matrixdata2[i] = dis(gen);
    }
    /*
    fill(matrixdata, matrixdata + (n * m), 0.1);
    fill(matrixdata2, matrixdata2 + (n1 * m1), 0.2);
    */
    for (int i = 0; i < n; i++) {
        matrix[i] = &matrixdata[i * m];
        matrixRes[i] = &matrixdataRes[i * m1];
    }

    for (int i = 0; i < n1; i++) {
        matrix2[i] = &matrixdata2[i * m1];
    }

    //Proceso de multiplicación de matrices
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m1; j++) {
            matrixRes[i][j] = 0;
            for (int k = 0; k < m; k++) {
                matrixRes[i][j] += matrix[i][k] * matrix2[k][j];
            }
        }
    }

    auto end = chrono::high_resolution_clock::now();
    chrono::duration<double> duration = end - start;
    cout << "El elemento (0, 0) es = " << matrixRes[0][0] << endl;
    cout << "El elemento (0, m) es = " << matrixRes[0][m1 - 1] << endl;
    cout << "El elemento (n, 0) es = " << matrixRes[n - 1][0] << endl;
    cout << "El elemento (n, m) es = " << matrixRes[n - 1][m1 - 1] << endl;
    cout << "El tiempo de ejecución fue de: " << duration.count() << endl;

    //Se libera la memoria utilizada
    delete[] matrixdata;
    delete[] matrixdata2;
    delete[] matrixdataRes;

    delete[] matrix;
    delete[] matrix2;
    delete[] matrixRes;
    return 0;
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
