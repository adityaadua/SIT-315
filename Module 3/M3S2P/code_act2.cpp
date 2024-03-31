#include <chrono>
#include <cstdlib>
#include <iostream>
#include <time.h>
#include <mpi.h>

using namespace std::chrono;
using namespace std;

void randomVector(int vector[], int size) {
    for (int i = 0; i < size; i++) {
        vector[i] = rand() % 100;
    }
}

int main(int argc, char *argv[]) {
    MPI_Init(&argc, &argv);
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    unsigned long total_size = 100000000;
    srand(time(0));
    int *v1, *v2, *v3_local;
    auto start = high_resolution_clock::now();
    v1 = (int *)malloc(total_size * sizeof(int));
    v2 = (int *)malloc(total_size * sizeof(int));
    v3_local = (int *)malloc(total_size / size * sizeof(int)); // local portion of v3

    randomVector(v1, total_size);
    randomVector(v2, total_size);

    // Scatter v1 and v2 to all processes
    MPI_Scatter(v1, total_size / size, MPI_INT, v1, total_size / size, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Scatter(v2, total_size / size, MPI_INT, v2, total_size / size, MPI_INT, 0, MPI_COMM_WORLD);

    // Local addition
    for (int i = 0; i < total_size / size; i++) {
        v3_local[i] = v1[i] + v2[i];
    }

    // Gather results back to v1
    MPI_Gather(v3_local, total_size / size, MPI_INT, v1, total_size / size, MPI_INT, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        auto stop = high_resolution_clock::now();
        auto duration = duration_cast<microseconds>(stop - start);
        cout << "Time taken by function: " << duration.count() << " microseconds" << endl;

        // Calculate total sum using MPI_Reduce
        int total_sum;
        MPI_Reduce(v1, &total_sum, total_size / size, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
        cout << "Total sum of all elements in v1: " << total_sum << endl;
    }

    MPI_Finalize();
    return 0;
}
