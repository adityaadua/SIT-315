#include <stdio.h>
#include <string.h>
#include <mpi.h>

int main(int argc, char *argv[]) {
    int rank, size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (rank == 0) { // Master process
        char message[] = "Hello World!";
        for (int dest = 1; dest < size; dest++) {
            MPI_Send(message, strlen(message) + 1, MPI_CHAR, dest, 0, MPI_COMM_WORLD);
        }
    } else { // Worker processes
        char recv_message[100];
        MPI_Recv(recv_message, 100, MPI_CHAR, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        printf("Rank %d received: %s\n", rank, recv_message);
    }

    MPI_Finalize();
    return 0;
}
