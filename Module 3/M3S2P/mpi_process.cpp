#include <mpi.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[]) {
    int rank, size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // The master process has rank 0
    if (rank == 0) {
        // Master process
        char message[20] = "Hello World!";
        for (int i = 1; i < size; i++) { // Start from 1 since 0 is the master
            MPI_Send(message, strlen(message)+1, MPI_CHAR, i, 0, MPI_COMM_WORLD);
        }
    } else {
        // Worker processes
        char receivedMessage[20];
        MPI_Recv(receivedMessage, 20, MPI_CHAR, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        printf("Process %d received message: %s\n", rank, receivedMessage);
    }

    MPI_Finalize();
    return 0;
}