#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "mpi.h"
#include "string.h"

#define size 2000                                             // The grid size - remember that the maximum integer value is 2147483647
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
bool is_satisfied(int **grid, char *agents, int start_row, int start_column, int total_num_rows, int total_num_cols, int x, int y);
void optimize_agents(int rank, int workers, int **grid, char *agents, int start_row, int start_column, int num_rows, int num_cols);
void move_agent(int **grid, int start_row, int start_column, int num_rows, int num_cols, int row, int col);
bool has_free_cells(int **grid, int start_row, int start_column, int num_rows, int num_cols);
int get_num_rows_to_analyze(int rank, int workers, int num_rows);
int get_start_row_to_analyze(int rank, int workers);
int get_start_row_of_worker(int rank, int workers);
int get_num_rows_of_worker(int rank, int workers);
bool all_agents_are_satisfied(int **grid, char *agents, int num_cols, int num_rows);

int main(int argc, char *argv[])
{
    int rank, processors, workers = 0, num_rows = 0, finished = 0, start_row = 0;
    int **grid;
    char *agents;

    double dt_start, dt_end;
    MPI_Status status;
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &processors);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    workers = processors - 1;
    bool all_satisfied = false;

    dt_start = MPI_Wtime();
    if (size < workers || processors < 2) // if less than 3 processors, or I cannot split the grid between the workers, skip
    {
        MPI_Abort(MPI_COMM_WORLD, 911);
        MPI_Finalize();
        return 0;
    }

    if (rank == MASTER_RANK) // master
    {
        // printf("The grid size is: %dx%d", size, size);
        // printf("\nThe percentage threshold of empty cells is: %d", empty_perc);
        // printf("\nThe percentage threshold of agent satisfaction is: %d", t);
        // printf("\nThe max rounds number is: %d", max_rounds);
        // printf("\nThe number of agents is: %d", num_agents);

        grid = allocate_grid(size, size);
        agents = allocate_agents(num_agents);
        initialize_grid(grid);
        initialize_agents(grid, agents);
        // print_grid(grid, agents, 0, 0, size, size);

        int rows_pointer;
        for (int round = 0; round < max_rounds && !all_satisfied; round++)
        {
            // printf("\n\nRound #%d started.", round);
            MPI_Request *requests = (MPI_Request *)malloc(workers * sizeof(MPI_Request));
            start_row = 0;
            rows_pointer = 0;
            for (int i = 1; i <= workers; i++)
            {
                start_row = rows_pointer;
                if (i > 1)
                {
                    start_row--;
                }
                MPI_Isend(&(finished), 1, MPI_INT, i, MESSAGE_TAG, MPI_COMM_WORLD, &(requests[i - 1]));
                num_rows = get_num_rows_of_worker(i, workers);
                MPI_Isend(&(grid[start_row][0]), (num_rows * size), MPI_INT, i, MESSAGE_TAG, MPI_COMM_WORLD, &(requests[i - 1]));
                MPI_Isend(&(agents[0]), num_agents, MPI_CHAR, i, MESSAGE_TAG, MPI_COMM_WORLD, &(requests[i - 1]));
                rows_pointer += (size / workers);
            }

            MPI_Waitall(workers, requests, MPI_STATUSES_IGNORE);

            start_row = 0;
            num_rows = 0;
            for (int i = 1; i <= workers; i++)
            {
                num_rows = get_num_rows_to_analyze(i, workers, get_num_rows_of_worker(i, workers));
                MPI_Recv(&(grid[start_row][0]), (num_rows * size), MPI_INT, i, MESSAGE_TAG, MPI_COMM_WORLD, &status);
                start_row += (size / workers);
            }
            MPI_Free_mem(requests);

            // print_grid(grid, agents, 0, 0, size, size);
            all_satisfied = all_agents_are_satisfied(grid, agents, size, size);
        }

        MPI_Request *requests = (MPI_Request *)malloc(workers * sizeof(MPI_Request));
        finished = 1;
        for (int i = 1; i <= workers; i++)
        {
            MPI_Isend(&(finished), 1, MPI_INT, i, MESSAGE_TAG, MPI_COMM_WORLD, &(requests[i - 1]));
        }
    }
    else // worker
    {
        MPI_Recv(&(finished), 1, MPI_INT, MASTER_RANK, MESSAGE_TAG, MPI_COMM_WORLD, &status);
        while (finished == 0)
        {
            num_rows = get_num_rows_of_worker(rank, workers);
            grid = allocate_grid(num_rows, size);
            agents = allocate_agents(num_agents);
            MPI_Recv(&(grid[0][0]), (num_rows * size), MPI_INT, MASTER_RANK, MESSAGE_TAG, MPI_COMM_WORLD, &status);
            MPI_Recv(&(agents[0]), num_agents, MPI_CHAR, MASTER_RANK, MESSAGE_TAG, MPI_COMM_WORLD, &status);
            start_row = get_start_row_to_analyze(rank, workers);
            num_rows = get_num_rows_to_analyze(rank, workers, num_rows);
            optimize_agents(rank, workers, grid, agents, start_row, 0, num_rows, size);
            MPI_Send(&(grid[start_row][0]), (num_rows * size), MPI_INT, MASTER_RANK, MESSAGE_TAG, MPI_COMM_WORLD);
            MPI_Recv(&(finished), 1, MPI_INT, MASTER_RANK, MESSAGE_TAG, MPI_COMM_WORLD, &status);
        }
    }

    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Free_mem(grid);
    MPI_Free_mem(agents);
    MPI_Finalize();
    dt_end = MPI_Wtime();

    if (rank == 0)
    {
        printf("All agents are satisfied: %s \nTime in second = %f\n", (all_satisfied ? "Yes" : "No"), (dt_end - dt_start));
    }

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
    for (int i = start_row; i < rows + start_row; i++)
    {
        for (int j = start_column; j < cols + start_column; j++)
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

bool is_satisfied(int **grid, char *agents, int start_row, int start_column, int total_num_rows, int total_num_cols, int x, int y)
{
    if (grid[x][y] == -1)
    {
        return true;
    }
    int i, j, neighbors = 1, siblings = 0;

    for (i = x - 1; i <= x + 1; i++)
    {
        if (i < 0 || i > total_num_rows - 1)
        {
            continue;
        }
        for (j = y - 1; j <= y + 1; j++)
        {
            if (j < 0 || j > total_num_cols - 1 || (i == x && j == y))
            {
                continue;
            }
            if (agents[grid[x][y]] == agents[grid[i][j]])
            {
                siblings++;
            }
            if (grid[i][j] != -1)
            {
                neighbors++;
            }
        }
    }
    if ((siblings * 100) / neighbors >= t)
    {
        return true;
    }
    return false;
}

int get_start_row_to_analyze(int rank, int workers)
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
int get_num_rows_to_analyze(int rank, int workers, int num_rows)
{
    if (rank == 1)
    {
        if (rank == workers)
        {
            return num_rows;
        }
        return num_rows - 1;
    }
    else if (rank == workers)
    {
        return num_rows - 1;
    }
    else
    {
        return num_rows - 2;
    }
}

void optimize_agents(int rank, int workers, int **grid, char *agents, int start_row, int start_column, int num_rows, int num_cols)
{
    if (!has_free_cells(grid, start_row, start_column, num_rows, num_cols))
    {
        return;
    }
    for (int i = 0; i < num_rows; i++)
    {
        for (int j = 0; j < num_cols; j++)
        {
            if (grid[i + start_row][j + start_column] == -1)
            {
                continue;
            }
            if (!is_satisfied(grid, agents, start_row, start_column, get_num_rows_of_worker(rank, workers), num_cols, i + start_row, j + start_column))
            {
                move_agent(grid, start_row, start_column, num_rows, num_cols, i + start_row, j + start_column);
            }
        }
    }
}

bool has_free_cells(int **grid, int start_row, int start_column, int num_rows, int num_cols)
{
    for (int i = 0; i < num_rows; i++)
    {
        for (int j = 0; j < num_cols; j++)
        {
            if (grid[i + start_row][j + start_column] == -1)
            {
                return true;
            }
        }
    }
    return false;
}

void move_agent(int **grid, int start_row, int start_column, int num_rows, int num_cols, int row, int col)
{
    int x, y;
    do
    {
        x = (rand() % (num_rows)) + start_row;
        y = (rand() % (num_cols)) + start_column;
    } while (grid[x][y] != -1);
    grid[x][y] = grid[row][col];
    grid[row][col] = -1;
    // printf("\nThe agent in (%d,%d) is not satisfied and has been moved on (%d,%d)", row, col, x, y);
}

int get_num_rows_of_worker(int rank, int workers)
{
    if (rank == 1)
    {
        if (rank == workers)
        {
            return size;
        }
        return (size / workers) + 1;
    }
    else if (rank == workers)
    {
        return (size / workers) + (size % workers) + 1;
    }
    else
    {
        return (size / workers) + 2;
    }
}

int get_start_row_of_worker(int rank, int workers)
{
    if (rank == 1)
    {
        return 0;
    }
    else if (rank == workers)
    {
        return ((size / workers) * (rank - 1)) + (size % workers);
    }
    else
    {
        return (size / workers) * (rank - 1);
    }
}

bool all_agents_are_satisfied(int **grid, char *agents, int num_cols, int num_rows)
{
    for (int i = 0; i < num_rows; i++)
    {
        for (int j = 0; j < num_cols; j++)
        {
            if (!is_satisfied(grid, agents, 0, 0, num_rows, num_cols, i, j))
            {
                return false;
            }
        }
    }
    return true;
}