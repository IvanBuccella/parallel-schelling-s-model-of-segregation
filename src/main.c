#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#ifdef _WIN32
#include <sys/time.h>
#else
#include <time.h>
#endif
#include "mpi.h"

#define size 10                                               // The grid size - remember that the maximum integer value is 2147483647
#define empty_perc 30                                         // The percentage threshold of empty cells
#define t 30                                                  // The percentage threshold of agent satisfaction
#define max_rounds 500                                        // The max steps number for satisfing the agents
#define num_agents ((size * size) * (100 - empty_perc)) / 100 // The number of agents to be satisfied
#define MESSAGE_TAG 50
#define MASTER_RANK 0

#define CHAR_BLUE 'B'
#define CHAR_RED 'R'
#define CHAR_EMPTY '-'

char **grid;
int **agents;

char **allocate_grid(int rows, int cols);
int **allocate_agents();
void print_grid(char **grid, int rows, int cols);
void initialize_grid();
void initialize_agents();

int main(int argc, char *argv[])
{
    int rank, processors, workers = 0, num_rows = 0;

    double dt_start, dt_end;
    MPI_Status status;
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &processors);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    workers = processors - 1;

    dt_start = MPI_Wtime();
    if (processors < 3) // if less than 2 processors, skip
    {
        MPI_Abort(MPI_COMM_WORLD, 911);
        MPI_Finalize();
        return 0;
    }

    if (rank == MASTER_RANK) // master
    {
        printf("The grid size is: %dx%d", size, size);
        printf("\nThe percentage threshold of empty cells is: %d", empty_perc);
        printf("\nThe percentage threshold of agent satisfaction is: %d", t);
        printf("\nThe max rounds number is: %d", max_rounds);
        printf("\nThe number of agents is: %d", num_agents);

        initialize_grid();
        initialize_agents();
        // print_grid(grid, size, size);

        int start_row;

        for (int round = 0; round < 1; round++) // Add max rounds && is_satisfied check
        {
            printf("\n\nRound #%d started.", round);
            MPI_Request *requests = (MPI_Request *)malloc(workers * sizeof(MPI_Request));
            start_row = 0;
            for (int i = 1; i <= workers; i++)
            {
                num_rows = size / workers;
                if (i == workers)
                {
                    num_rows += size % workers;
                }
                MPI_Isend(&(grid[start_row][0]), (num_rows * size), MPI_CHAR, i, MESSAGE_TAG, MPI_COMM_WORLD, &(requests[i - 1]));
                start_row += num_rows;
            }
            MPI_Waitall(workers, requests, MPI_STATUSES_IGNORE);
            MPI_Free_mem(requests);
        }

        MPI_Free_mem(grid);
    }
    else // worker
    {
        num_rows = size / workers;
        if (rank == workers)
        {
            num_rows += size % workers;
        }
        grid = allocate_grid(num_rows, size);
        MPI_Recv(&(grid[0][0]), (num_rows * size), MPI_CHAR, MASTER_RANK, MESSAGE_TAG, MPI_COMM_WORLD, &status);
        // print_grid(grid, num_rows, size);
        MPI_Free_mem(grid);
    }

    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Finalize();
    dt_end = MPI_Wtime();

    if (rank == 0)
        printf("\nTime in second = %f\n", dt_end - dt_start);

    return 0;
}

void initialize_grid()
{
    grid = allocate_grid(size, size);
    int i, j;
    for (i = 0; i < size; i++)
    {
        for (j = 0; j < size; j++)
        {
            grid[i][j] = CHAR_EMPTY;
        }
    }
}

void initialize_agents()
{
    agents = allocate_agents();
    int i, j, location[2];
    for (i = 0; i < num_agents; i++)
    {
        agents[i][0] = -1;
        agents[i][1] = -1;
    }
    for (i = 0; i < num_agents; i++)
    {
        do
        {
            location[0] = rand() % size;
            location[1] = rand() % size;
        } while (grid[location[0]][location[1]] != CHAR_EMPTY);
        agents[i][0] = location[0];
        agents[i][1] = location[1];
        if ((i % 2) == 0)
        {
            grid[location[0]][location[1]] = CHAR_BLUE;
        }
        else
        {
            grid[location[0]][location[1]] = CHAR_RED;
        }
    }
}

void print_grid(char **grid, int rows, int cols)
{
    printf("\n\n");
    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < cols; j++)
        {
            printf("%c", grid[i][j]);
            printf("     ");
        }
        printf("\n");
    }
}

char **allocate_grid(int rows, int cols)
{
    char *data = (char *)malloc(rows * cols * sizeof(char));
    char **array = (char **)malloc(rows * sizeof(char *));
    for (int i = 0; i < rows; i++)
        array[i] = &(data[cols * i]);

    return array;
}

int **allocate_agents()
{
    int *data = (int *)malloc(num_agents * 2 * sizeof(int));
    int **array = (int **)malloc(num_agents * sizeof(int *));
    for (int i = 0; i < num_agents; i++)
        array[i] = &(data[2 * i]);

    return array;
}