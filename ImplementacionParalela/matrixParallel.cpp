#include <mpi.h>
#include <iostream>
#include <random>

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
    int rank, size, filas, filas1, columnas, columnas1;
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> dis(1, 300);

    //Se piden 4 argumentos filas de A, columnas de A, filas de B, columnas de B
    if (argc == 5 ) {
        filas = atoi(argv[1]);
        columnas = atoi(argv[2]);
        filas1 = atoi(argv[3]);
        columnas1 = atoi(argv[4]);
    } else {
        cout << "ERROR. El número de parámetros ingresados no es correcto. Debe indicar (filasA columnasA filasB columnasB)" << endl;
        return 0;
    }
    
    //Se verifica que efectivamente se pueda realizar la multiplicación
    if (columnas != filas1) {
        cout << "ERROR. Las matrices deben ser de tamaño n x m y m x k respectivamente." << endl;
        return 0;
    }

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    double start = MPI_Wtime();
    //Utilizados para el scatterv y el gatherv. Vector de elementos enviados y de desplazamientos.
    double computingTime = MPI_Wtime();

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
    
    //Declaro e inicializo los datos para las matrices B y las filas de A que cada proceso recibirá
    float* recvMatrixData = new float[sendsizes[rank]];
    float* matrixdata1 = new float[filas1 * columnas1];
    float** matrix1 = new float*[filas1];
    //Declaro variables utilizadas para tomar los tiempos de comunicación
    double scatterTimeStart;
    double scatterTimeEnd;
    double bcastTimeStart;
    double bcastTimeEnd;
    double gatherTimeStart;
    double gatherTimeEnd;
    double end;

    if (rank == 0) {
        float* matrixdata = new float[filas * columnas];
        float** matrix = new float*[filas];
        
        //Inicializo las matrices con números random
        for (int i = 0; i < filas * columnas; i++) {
            matrixdata[i] = dis(gen);
        }
        for (int i = 0; i < filas1 * columnas1; i++) {
            matrixdata1[i] = dis(gen);
        }
        /*
        fill(matrixdata, matrixdata + (filas * columnas), 0.1);
        fill(matrixdata1, matrixdata1 + (filas1 * columnas1), 0.2);
        */
        //Formo las matrices
        for (int i = 0; i < filas; ++i) {
            matrix[i] = &matrixdata[i * columnas];
        }

        for (int i = 0; i < filas1; ++i) {
            matrix1[i] = &matrixdata1[i * columnas1];
        }
        scatterTimeStart = MPI_Wtime();
        //Realizo el scatter de las filas de A
        MPI_Scatterv(matrixdata, sendsizes, displs, MPI_FLOAT, recvMatrixData, sendsizes[rank], MPI_FLOAT, 0, MPI_COMM_WORLD);
        scatterTimeEnd = MPI_Wtime();
        //Libero la memoria de la matriz que ya fue utilizada
        delete[] matrix;
        delete[] matrixdata;
    } else {
        scatterTimeStart = MPI_Wtime();
        MPI_Scatterv(nullptr, sendsizes, displs, MPI_FLOAT, recvMatrixData, sendsizes[rank], MPI_FLOAT, 0, MPI_COMM_WORLD);
        scatterTimeEnd = MPI_Wtime();
    }
    bcastTimeStart = MPI_Wtime();
    //Realizo el broadcast de la matriz B
    MPI_Bcast(matrixdata1, filas1 * columnas1, MPI_FLOAT, 0, MPI_COMM_WORLD);
    bcastTimeEnd = MPI_Wtime();

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

        gatherTimeStart = MPI_Wtime();
        //Realizo el gather de todos los resultados obtenidos
        MPI_Gatherv(resMatrixAuxData, rowCount * columnas1, MPI_FLOAT, matrixdatarta, recvcounts, displs_rta, MPI_FLOAT, 0, MPI_COMM_WORLD);
        end = MPI_Wtime();
        gatherTimeEnd = MPI_Wtime();

        //cout << "El elemento (0, 0) es = " << matrixrta[0][0] << endl;
        //cout << "El elemento (0, m) es = " << matrixrta[0][columnas1 - 1] << endl;
        //cout << "El elemento (n, 0) es = " << matrixrta[filas - 1][0] << endl;
        //cout << "El elemento (n, m) es = " << matrixrta[filas - 1][columnas1 - 1] << endl;
        cout << "El tiempo de ejecución fue de " << end - start << " segundos." << endl;
        //Libero la memoria utilizada en el proceso 0
        delete[] matrixrta;
        delete[] matrixdatarta;
        delete[] recvcounts;
        delete[] displs_rta;

    } else {
        gatherTimeStart = MPI_Wtime();
        MPI_Gatherv(resMatrixAuxData, rowCount * columnas1, MPI_FLOAT, nullptr, nullptr, nullptr, MPI_FLOAT, 0, MPI_COMM_WORLD);
        gatherTimeEnd = MPI_Wtime();
        end = MPI_Wtime();
    }
    //Muestra el tiempo de comunicación y tiempo de cómputo de cada proceso
    double commTime = (gatherTimeEnd - gatherTimeStart) + (bcastTimeEnd - bcastTimeStart) + (scatterTimeEnd - scatterTimeStart);
    cout << "Tiempo de cómputo del proceso " << rank <<": " << end - start - commTime << ". Tiempo de comunicación: " << commTime << endl;

    //Libero la memoria utilizada en cada proceso
    delete[] sendsizes;
    delete[] displs;
    delete[] recvMatrixData;
    delete[] matrixdata1;
    delete[] matrix1;
    delete[] resMatrixAux;
    delete[] resMatrixAuxData;
    delete[] recvMatrix;

    MPI_Finalize();
    return 0;
}

