#include <iostream>
#include <ctime>
#include <fstream>
#include <random>
#include <pthread.h>

using namespace std;

struct ThreadArgs {
    int **A;
    int **B;
    int **C;
    int N;
    int threadID;
    int numThreads;
};

void generateRandomMatrix(int **&matrix, int N) {
    matrix = new int*[N];
    for (int i = 0; i < N; ++i) {
        matrix[i] = new int[N];
        for (int j = 0; j < N; ++j) {
            matrix[i][j] = rand() % 100; // Random values between 0 and 99
        }
    }
}

void *matrixMultiplication(void *arguments) {
    ThreadArgs *args = (ThreadArgs*) arguments;
    int N = args->N;
    int startRow = (N / args->numThreads) * args->threadID;
    int endRow = (args->threadID == args->numThreads - 1) ? N : (N / args->numThreads) * (args->threadID + 1);

    for (int i = startRow; i < endRow; ++i) {
        for (int j = 0; j < N; ++j) {
            args->C[i][j] = 0;
            for (int k = 0; k < N; ++k) {
                args->C[i][j] += args->A[i][k] * args->B[k][j];
            }
        }
    }

    pthread_exit(NULL);
}

void writeMatrixToFile(int **matrix, int N, const string& filename) {
    ofstream outputFile(filename);
    if (!outputFile.is_open()) {
        cerr << "Unable to open file " << filename << endl;
        return;
    }
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) {
            outputFile << matrix[i][j] << " ";
        }
        outputFile << endl;
    }
    outputFile.close();
}

int main() {
    srand(time(nullptr)); // Seed for random number generation

    int N = 400; // Size of matrices (N x N)
    int numThreads = 4; // Number of threads

    int **A, **B, **C;

    // Generate random matrices A and B
    generateRandomMatrix(A, N);
    generateRandomMatrix(B, N);

    // Allocate memory for result matrix C
    C = new int*[N];
    for (int i = 0; i < N; ++i) {
        C[i] = new int[N];
    }

    pthread_t threads[numThreads];
    ThreadArgs threadArgs[numThreads];
    for (int i = 0; i < numThreads; ++i) {
        threadArgs[i].A = A;
        threadArgs[i].B = B;
        threadArgs[i].C = C;
        threadArgs[i].N = N;
        threadArgs[i].threadID = i;
        threadArgs[i].numThreads = numThreads;
    }

    // Perform matrix multiplication using threads
    clock_t startTime = clock();
    for (int i = 0; i < numThreads; ++i) {
        pthread_create(&threads[i], NULL, matrixMultiplication, (void*)&threadArgs[i]);
    }

    // Join threads
    for (int i = 0; i < numThreads; ++i) {
        pthread_join(threads[i], NULL);
    }
    clock_t endTime = clock();

    double executionTime = double(endTime - startTime) / CLOCKS_PER_SEC;
    writeMatrixToFile(C, N, "matrix.txt");

    // Cleanup
    for (int i = 0; i < N; ++i) {
        delete[] A[i];
        delete[] B[i];
        delete[] C[i];
    }
    delete[] A;
    delete[] B;
    delete[] C;

    cout << "Execution time: " << executionTime << " seconds" << endl;

    return 0;
}
