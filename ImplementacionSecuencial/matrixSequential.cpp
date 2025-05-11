#include <iostream>
#include <chrono>

using namespace std;

void printMatrix(float** matrix, int n, int m);

int main(int argc, char** argv) {
    //n y m son las filas y columnas de la matriz 1. n1 y m1 son las filas y columnas de la matriz 2.
    int n, m, n1, m1;
   
    if (argc == 2 ) {
        n = atoi(argv[1]);
        if (n != 300 && n != 1000 && n != 3000) {
            cout << "ERROR. El parámetro ingresado no es correcto. Debe ingresar 300/1000/3000" << endl;
            return 0;
        }
        m = n;
        m1 = m;
        n1 = n;
    } else {
        cout << "ERROR. El número de parámetros ingresados no es correcto. Debe indicar 300/1000/3000." << endl;
        return 0;
    }

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
        fill(matrix[i], matrix[i] + m, 0.1);
        matrixRes[i] = new float[m1];
    }
    for (int i = 0; i < n1; i++) {
        matrix2[i] = new float[m1];
        fill(matrix2[i], matrix2[i] + m1, 0.2);
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