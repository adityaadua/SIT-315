#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <omp.h>

#define DIM 1000
#define MAX_RAND 10

int matrix1[DIM][DIM];
int matrix2[DIM][DIM];
int res[DIM][DIM];

void head(int proc, int start, int end); //head process 
void node(int proc, int start, int end); //node processes
void create(int matrix[DIM][DIM]); //create matrix
void output(int matrix[DIM][DIM]); //output matrix

/* holding the values */
struct vari
{
    int proc_rank, proc, start, end, num_threads;
    double t_start, t_stop;
};

vari val = {0, 0, 0, 0, 2, 0.0, 0.0};

int main(int argc, char *argv[])
{

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &val.proc_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &val.proc);

    val.start = val.proc_rank * DIM / val.proc; 
    val.end = ((val.proc_rank + 1) * DIM / val.proc);

    if (val.proc_rank == 0){
        val.t_start = omp_get_wtime();
    }

    if (val.proc_rank == 0)
    {
        head(val.proc, val.start, val.end);

    }
    else
    {
        node(val.proc, val.start, val.end);
    }

    if (val.proc_rank == 0){
        val.t_stop= omp_get_wtime();
        printf("MPI Matrix Multiplication Performance with OpenMP \n");
        printf("Dimension: %d \n", DIM);
        printf("Processes: %d \n", val.proc);
        printf("Threads: %d \n", val.num_threads);
        printf("Run time: %f \n", val.t_stop-val.t_start);
        if(DIM<=10){
            printf("First matrix: \n");
            output(matrix1);
            printf("Second matrix: \n"); 
            output(matrix2);
            printf("Result: \n");  
            output(res);
        }

    }

    MPI_Finalize();
    return 0;
}

void head(int proc, int start, int end){
    create(matrix1); create(matrix2);

    MPI_Bcast(matrix2, DIM * DIM, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Scatter(&matrix1[0][0], DIM * DIM / proc, MPI_INT, MPI_IN_PLACE, DIM * DIM / proc, MPI_INT, 0, MPI_COMM_WORLD);


    #pragma omp parallel num_threads(val.num_threads)
    {
        #pragma omp for
        for (int i = start; i < end; i++)
            for (int j = 0; j < DIM; j++)
            {
                res[i][j] = 0;
                for (int k = 0; k < DIM; k++)
                    res[i][j] += matrix1[i][k] * matrix2[k][j];
            }
    }

    MPI_Gather(MPI_IN_PLACE, DIM*DIM/proc, MPI_INT, &res[0][0], DIM*DIM/proc, MPI_INT, 0, MPI_COMM_WORLD);

}

void node(int proc, int start, int end){

    MPI_Bcast(matrix2, DIM * DIM, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Scatter(&matrix1[0][0], DIM * DIM / proc, MPI_INT, &matrix1[start], DIM * DIM / proc, MPI_INT, 0, MPI_COMM_WORLD);

    #pragma omp parallel num_threads(val.num_threads)
    {
        #pragma omp for
        for (int i = start; i < end; i++)
            for (int j = 0; j < DIM; j++)
            {
                res[i][j] = 0;
                for (int k = 0; k < DIM; k++)
                    res[i][j] += matrix1[i][k] * matrix2[k][j];
            }
    }
     MPI_Gather(&res[start], DIM*DIM/proc, MPI_INT, &res, DIM*DIM/proc, MPI_INT, 0, MPI_COMM_WORLD);

}

void create(int matrix[DIM][DIM])
{
  for (int i=0; i<DIM; i++)
    for (int j=0; j<DIM; j++)
      matrix[i][j] = rand() % MAX_RAND;
}

void output(int matrix[DIM][DIM])
{
  for (int i=0; i<DIM; i++) {
    for (int j=0; j<DIM; j++)
      std::cout<<matrix[i][j]<<"\t";
    std::cout<<"\n";
  }
    std::cout<<"\n";
}
