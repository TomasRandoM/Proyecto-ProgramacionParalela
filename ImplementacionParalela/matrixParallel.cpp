#include <mpi.h>
#include <iostream>

using namespace std;

void printMatrix(float** matrix, int n, int m) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            cout << matrix[i][j] << " ";
        }
        cout << endl;
    }
}

int main(int argc, char** argv) {
    int rank, size;
    int filas = 6, columnas = 3;
    int filas1 = 3, columnas1 = 2;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    //Utilizados para el scatterv y el gatherv. Vector de elementos enviados y de desplazamientos.
    int* sendsizes = new int[size];
    int* displs = new int[size];

    //Calculo cuantas filas le tocará a cada proceso
    int filasProc = filas / size;
    //Calculo las sobrantes en caso de que no sean multiplos con la cantidad de procesos
    int left = filas % size;

    //Relleno el vector sendsizes con la cantidad de elementos que recibirá cada proceso y el vector de desplazamientos
    int aux = 0; 
    for (int i = 0; i < size; i++) {
        if (i < left) {
            sendsizes[i] = (filasProc + 1) * columnas;
        } else {
            sendsizes[i] = filasProc * columnas;
        }
        displs[i] = aux;
        aux += sendsizes[i];
    }
    
    //Bcast para otra versión
    //MPI_Bcast(sendsizes, size, MPI_INT, 0, MPI_COMM_WORLD);
    //MPI_Bcast(displs, size, MPI_INT, 0, MPI_COMM_WORLD);

    //Declaro e inicializo los datos para las matrices B y las filas de A que cada proceso recibirá
    float* recvMatrixData = new float[sendsizes[rank]];
    float* matrixdata1 = new float[filas1 * columnas1];
    float** matrix1 = new float*[filas1];

    if (rank == 0) {
        float* matrixdata = new float[filas * columnas];
        float** matrix = new float*[filas];
        
        for (int i = 0; i < filas; ++i) {
            matrix[i] = &matrixdata[i * columnas];
        }

        for (int i = 0; i < filas1; ++i) {
            matrix1[i] = &matrixdata1[i * columnas1];
        }

        //inicializo las matrices
        for (int i = 0; i < filas; ++i) {
            for (int j = 0; j < columnas; ++j) {
                matrix[i][j] = (i+1)*(j+1); 
            }
        }
        for (int i = 0; i < filas1; ++i) {
            for (int j = 0; j < columnas1; ++j) {
                matrix1[i][j] = 2*(i+1)*(j+1); 
            }
        }
        printMatrix(matrix, filas, columnas);
        printMatrix(matrix1, filas1, columnas1);


        //Realizo el scatter de las filas de A
        MPI_Scatterv(matrixdata, sendsizes, displs, MPI_FLOAT, recvMatrixData, sendsizes[rank], MPI_FLOAT, 0, MPI_COMM_WORLD);
    } else {
        MPI_Scatterv(nullptr, sendsizes, displs, MPI_FLOAT, recvMatrixData, sendsizes[rank], MPI_FLOAT, 0, MPI_COMM_WORLD);
    }
    
    //Realizo el broadcast de la matriz B
    MPI_Bcast(matrixdata1, filas1 * columnas1, MPI_FLOAT, 0, MPI_COMM_WORLD);

    //Formo la matriz A recibida y la matriz auxiliar para almacenar el resultado parcial
    int rowCount = sendsizes[rank] / columnas;
    float** resMatrixAux = new float*[rowCount];
    float* resMatrixAuxData = new float[rowCount * columnas1];

    float** recvMatrix = new float*[rowCount];
    for (int i = 0; i < rowCount; ++i) {
        resMatrixAux[i] = &resMatrixAuxData[i * columnas1];
        recvMatrix[i] = &recvMatrixData[i * columnas];
    }
    
    //Formo la matriz B recibida
    if (rank != 0) {
        for (int i = 0; i < filas1; ++i) {
            matrix1[i] = &matrixdata1[i * columnas1];
        }
    }
    
    //Calculo de multiplicación
    for (int i = 0; i < rowCount; i++) {
        for (int j = 0; j < columnas1; j++) {
            resMatrixAux[i][j] = 0;
            for (int k = 0; k < columnas; k++) {
                resMatrixAux[i][j] += recvMatrix[i][k] * matrix1[k][j];
            }
        }
    }

    if (rank == 0) {
        //Formo la matriz resultado en el proceso 0
        float* matrixdatarta = new float[filas * columnas1];
        float** matrixrta = new float*[filas];

        for (int i = 0; i < filas; ++i) {
            matrixrta[i] = &matrixdatarta[i * columnas1];
        }

        int* recvcounts = new int[size]; //guarda cuantos elementos va a enviar cada proceso al proceso 0 en el gather
        int* displs_rta = new int[size]; //guarda los desplazamientos
        int aux2 = 0;
        for (int i = 0; i < size; ++i) {
            recvcounts[i] = (sendsizes[i] / columnas) * columnas1; //cantidad de elementos que va a mandar ese proceso.
            displs_rta[i] = aux2; //desplazamiento actual 
            aux2 += recvcounts[i];
        }

        //Realizo el gather de todos los resultados obtenidos
        MPI_Gatherv(resMatrixAuxData, rowCount * columnas1, MPI_FLOAT, matrixdatarta, recvcounts, displs_rta, MPI_FLOAT, 0, MPI_COMM_WORLD);

        cout << "Matriz Resultado:" << endl;
        printMatrix(matrixrta, filas, columnas1);

    } else {
        MPI_Gatherv(resMatrixAuxData, rowCount * columnas1, MPI_FLOAT, nullptr, nullptr, nullptr, MPI_FLOAT, 0, MPI_COMM_WORLD);

    }

    MPI_Finalize();
    return 0;
}

