#include <iostream>
#include <ctime>
#include <fstream>
#include <random>

using namespace std;

void generateRandomMatrix(int **&matrix, int N) {
    matrix = new int*[N];
    for (int i = 0; i < N; ++i) {
        matrix[i] = new int[N];
        for (int j = 0; j < N; ++j) {
            matrix[i][j] = rand() % 100; // Random values between 0 and 99
        }
    }
}

void multiplyMatrices(int **A, int **B, int **C, int N) {
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) {
            C[i][j] = 0;
            for (int k = 0; k < N; ++k) {
                C[i][j] += A[i][k] * B[k][j];
            }
        }
    }
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
    int **A, **B, **C;

    generateRandomMatrix(A, N);
    generateRandomMatrix(B, N);

    // Allocate memory for result matrix C
    C = new int*[N];
    for (int i = 0; i < N; ++i) {
        C[i] = new int[N];
    }

    // Perform matrix multiplication
    clock_t startTime = clock();
    multiplyMatrices(A, B, C, N);
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
