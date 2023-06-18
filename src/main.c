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

int **allocate_grid(int rows, int cols);
char *allocate_agents(int number);
void print_grid(int **grid, char *agents, int start_row, int start_column, int rows, int cols);
void initialize_grid(int **grid);
void initialize_agents(int **grid, char *agents);
bool is_satisfied(int **grid, char *agents, int grid_width, int grid_height, int x, int y);
void optimize_agents(int rank, int workers, int **grid, char *agents, int grid_width, int grid_height);
void move_agent(int **grid, int grid_width, int grid_height, int start_row, int start_column, int row, int col);
bool has_free_cells(int **grid, int grid_width, int grid_height, int start_row, int start_column);
int get_grid_height_of_worker(int rank, int workers, int num_rows);
int get_start_row_of_worker(int rank, int workers);

int main(int argc, char *argv[])
{
    int rank, processors, workers = 0, num_rows = 0;
    int **grid;
    char *agents;

    double dt_start, dt_end;
    MPI_Status status;
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &processors);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    workers = processors - 1;

    dt_start = MPI_Wtime();
    if (size < workers || processors < 3) // if less than 3 processors, or I cannot split the grid between the workers, skip
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

        grid = allocate_grid(size, size);
        agents = allocate_agents(num_agents);
        initialize_grid(grid);
        initialize_agents(grid, agents);
        print_grid(grid, agents, 0, 0, size, size);

        int start_row, rows_pointer;

        for (int round = 0; round < 1; round++) // Add max rounds && is_satisfied check
        {
            printf("\n\nRound #%d started.", round);
            MPI_Request *requests = (MPI_Request *)malloc(workers * sizeof(MPI_Request));
            start_row = 0;
            rows_pointer = 0;
            for (int i = 1; i <= workers; i++)
            {
                start_row = rows_pointer;
                if (i == 1)
                {
                    num_rows = (size / workers) + 1;
                }
                else if (i == workers)
                {
                    start_row--;
                    num_rows = (size / workers) + (size % workers) + 1;
                }
                else
                {
                    start_row--;
                    num_rows = (size / workers) + 2;
                }
                MPI_Isend(&(num_rows), 1, MPI_INT, i, MESSAGE_TAG, MPI_COMM_WORLD, &(requests[i - 1]));
                MPI_Isend(&(grid[start_row][0]), (num_rows * size), MPI_INT, i, MESSAGE_TAG, MPI_COMM_WORLD, &(requests[i - 1]));
                MPI_Isend(&(agents[0]), num_agents, MPI_CHAR, i, MESSAGE_TAG, MPI_COMM_WORLD, &(requests[i - 1]));
                rows_pointer += (size / workers);
            }
            MPI_Waitall(workers, requests, MPI_STATUSES_IGNORE);
            MPI_Free_mem(requests);
        }

        MPI_Free_mem(grid);
    }
    else // worker
    {
        MPI_Recv(&(num_rows), 1, MPI_INT, MASTER_RANK, MESSAGE_TAG, MPI_COMM_WORLD, &status);
        grid = allocate_grid(num_rows, size);
        agents = allocate_agents(num_agents);
        MPI_Recv(&(grid[0][0]), (num_rows * size), MPI_INT, MASTER_RANK, MESSAGE_TAG, MPI_COMM_WORLD, &status);
        MPI_Recv(&(agents[0]), num_agents, MPI_CHAR, MASTER_RANK, MESSAGE_TAG, MPI_COMM_WORLD, &status);
        optimize_agents(rank, workers, grid, agents, size, get_grid_height_of_worker(rank, workers, num_rows));
        MPI_Free_mem(grid);
    }

    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Finalize();
    dt_end = MPI_Wtime();

    if (rank == 0)
        printf("\nTime in second = %f\n", dt_end - dt_start);

    return 0;
}

void initialize_grid(int **grid)
{
    int i, j;
    for (i = 0; i < size; i++)
    {
        for (j = 0; j < size; j++)
        {
            grid[i][j] = -1;
        }
    }
}

void initialize_agents(int **grid, char *agents)
{
    int x, y;
    for (int i = 0; i < num_agents; i++)
    {
        do
        {
            x = rand() % size;
            y = rand() % size;
        } while (grid[x][y] != -1);
        grid[x][y] = i;
        if ((i % 2) == 0)
        {
            agents[i] = CHAR_BLUE;
        }
        else
        {
            agents[i] = CHAR_RED;
        }
    }
}

void print_grid(int **grid, char *agents, int start_row, int start_column, int rows, int cols)
{
    printf("\n\n");
    for (int i = start_row; i < rows; i++)
    {
        for (int j = start_column; j < cols; j++)
        {
            if (grid[i][j] == -1)
            {
                printf("%c", CHAR_EMPTY);
            }
            else
            {
                printf("%c", agents[grid[i][j]]);
            }
            printf("     ");
        }
        printf("\n");
    }
}

int **allocate_grid(int rows, int cols)
{
    int *data = (int *)malloc(rows * cols * sizeof(int));
    int **array = (int **)malloc(rows * sizeof(int *));
    for (int i = 0; i < rows; i++)
        array[i] = &(data[cols * i]);

    return array;
}

char *allocate_agents(int number)
{
    char *array = (char *)malloc(number * sizeof(char));
    return array;
}

bool is_satisfied(int **grid, char *agents, int grid_width, int grid_height, int x, int y)
{
    if (grid[x][y] == -1)
    {
        return false;
    }

    int i, j, neighbors = 0, siblings = 0;

    for (i = x - 1; i <= x + 1; i++)
    {
        if (i < 0 || i > grid_height - 1)
        {
            continue;
        }
        for (j = y - 1; j <= y + 1; j++)
        {
            if (j < 0 || j > grid_width - 1 || (i == x && j == y))
            {
                continue;
            }

            if (agents[grid[x][y]] == agents[grid[i][j]])
            {
                siblings++;
            }
            neighbors++;
        }
    }
    if ((siblings * 100) / neighbors >= t)
    {
        return true;
    }
    return false;
}

int get_start_row_of_worker(int rank, int workers)
{
    if (rank == 1)
    {
        return 0;
    }
    else if (rank == workers)
    {
        return 1;
    }
    else
    {
        return 1;
    }
}
int get_grid_height_of_worker(int rank, int workers, int num_rows)
{
    if (rank == 1)
    {
        return num_rows - 1;
    }
    else if (rank == workers)
    {
        return num_rows;
    }
    else
    {
        return num_rows - 1;
    }
}

void optimize_agents(int rank, int workers, int **grid, char *agents, int grid_width, int grid_height)
{
    int start_row = get_start_row_of_worker(rank, workers), start_column = 0;
    if (!has_free_cells(grid, grid_width, grid_height, start_row, start_column))
    {
        return;
    }
    for (int i = start_row; i < grid_height; i++)
    {
        for (int j = start_column; j < grid_width; j++)
        {
            if (grid[i][j] == -1)
            {
                continue;
            }
            // printf("\nIs Satisfied (%d,%d): %d", i, j, is_satisfied(grid, agents, grid_width, grid_height, i, j));
            if (!is_satisfied(grid, agents, grid_width, grid_height, i, j))
            {
                move_agent(grid, grid_width, grid_height, start_row, start_column, i, j);
            }
        }
    }
}

bool has_free_cells(int **grid, int grid_width, int grid_height, int start_row, int start_column)
{
    for (int i = start_row; i < grid_height; i++)
    {
        for (int j = start_column; j < grid_width; j++)
        {
            if (grid[i][j] == -1)
            {
                return true;
            }
        }
    }
    return false;
}

void move_agent(int **grid, int grid_width, int grid_height, int start_row, int start_column, int row, int col)
{
    int x, y;
    do
    {
        x = rand() % grid_height;
        y = rand() % grid_width;

    } while (grid[x][y] != -1 || x < start_row || y < start_column);
    grid[x][y] = grid[row][col];
    grid[row][col] = -1;
    // printf("\nThe agent in (%d,%d) is not satisfied and has been moved on (%d,%d)", row, col, x, y);
}
