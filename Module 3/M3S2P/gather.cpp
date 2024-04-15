#include <mpi.h>
#include <cstdlib>
#include <iostream>
#include <chrono>
#include <ctime>

using namespace std;
using namespace std::chrono;

void randomVector(int vector[], int size) {
    for (int i = 0; i < size; i++) {
        vector[i] = rand() % 100;
    }
}

int main(int argc, char *argv[]) {
    int rank, size;
    unsigned long global_size = 10000;
    int *v1 = nullptr, *v2 = nullptr, *v3 = nullptr, *local_v1, *local_v2, *local_v3;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    unsigned long local_size = global_size / size;

    if (rank == 0) {
        v1 = (int *)malloc(global_size * sizeof(int));
        v2 = (int *)malloc(global_size * sizeof(int));
        v3 = (int *)malloc(global_size * sizeof(int));
        srand(time(0));
        randomVector(v1, global_size);
        randomVector(v2, global_size);
    }

    auto start = high_resolution_clock::now();

    local_v1 = (int *)malloc(local_size * sizeof(int));
    local_v2 = (int *)malloc(local_size * sizeof(int));
    local_v3 = (int *)malloc(local_size * sizeof(int));

    MPI_Scatter(v1, local_size, MPI_INT, local_v1, local_size, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Scatter(v2, local_size, MPI_INT, local_v2, local_size, MPI_INT, 0, MPI_COMM_WORLD);

    for (int i = 0; i < local_size; i++) {
        local_v3[i] = local_v1[i] + local_v2[i];
    }

    cout << "Process " << rank << " added vectors: ";
    for (int i = 0; i < local_size; i++) {
        if (i < 10) {  // Print only the first 10 results
            cout << local_v1[i] << "+" << local_v2[i] << "=" << local_v3[i] << " ";
        }
    }
    cout << endl;

    MPI_Gather(local_v3, local_size, MPI_INT, v3, local_size, MPI_INT, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        auto stop = high_resolution_clock::now();
        auto duration = duration_cast<microseconds>(stop - start);
        cout << "Time taken by function: " << duration.count() << " microseconds" << endl;

        free(v1);
        free(v2);
        free(v3);
    }

    free(local_v1);
    free(local_v2);
    free(local_v3);

    MPI_Finalize();

    return 0;
}