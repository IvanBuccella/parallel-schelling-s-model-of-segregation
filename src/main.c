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

#define size 10       // The grid size
#define empty_perc 30 // The percentage threshold of empty cells
#define t 30          // The percentage threshold of agent satisfaction
#define CHAR_BLUE 'B'
#define CHAR_RED 'R'
#define CHAR_EMPTY '-'
int num_agents = (size * size) - ((size * size) * empty_perc / 100);
static char grid[size][size];
static int agents[size][2];

void print_grid();
void initialize_agents();
bool is_satisfied(int index);
void move_agent(int index);

int main()
{
    double start, end;

    printf("\nThe grid size is: %dx%d", size, size);
    printf("\nThe percentage threshold of empty cells is: %d", empty_perc);
    printf("\nThe percentage threshold of agent satisfaction is: %d", t);
    printf("\nNum Agents: %d", num_agents);

    start = MPI_Wtime();

    initialize_agents();
    print_grid();

    int i;
    for (i = 0; i < num_agents; i++)
    {
        if (!is_satisfied(i))
        {
            printf("\nThe agent in (%d,%d) is not satisfied", agents[i][0], agents[i][1]);
            move_agent(i);
        }
    }
    print_grid();

    end = MPI_Wtime();
    printf("\nTime in second = %f\n", end - start);

    return 0;
}

void initialize_agents()
{
    int i, j;
    int location[2];

    for (i = 0; i < size; i++)
    {
        for (j = 0; j < size; j++)
        {
            grid[i][j] = CHAR_EMPTY;
        }
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

void print_grid()
{
    int i, j;
    printf("\n");
    for (i = 0; i < size; i++)
    {
        for (j = 0; j < size; j++)
        {
            printf("%c", grid[i][j]);
            printf("     ");
        }
        printf("\n");
    }
}

bool is_satisfied(int index)
{
    int i, j, neighbors = 0, siblings = 0, row = agents[index][0], col = agents[index][1];
    for (i = row - 1; i <= row + 1; i++)
    {
        if (i < 0 || i > size - 1)
        {
            continue;
        }
        for (j = col - 1; j <= col + 1; j++)
        {
            if (j < 0 || j > size - 1 || (i == row && j == col))
            {
                continue;
            }

            if (grid[row][col] == grid[i][j])
            {
                siblings++;
            }
            neighbors++;
        }
    }
    int perc = (siblings * 100) / neighbors;
    // printf("\nrow: %d", row);
    // printf("\ncol: %d", col);
    // printf("\nNeighbors: %d", neighbors);
    // printf("\nSiblings: %d", siblings);
    // printf("\nNeighbors/Siblings: %d", perc);
    if (perc >= t)
    {
        return true;
    }
    return false;
}

void move_agent(int index)
{
    int location[2], row = agents[index][0], col = agents[index][1];
    do
    {
        location[0] = rand() % size;
        location[1] = rand() % size;
    } while (grid[location[0]][location[1]] != CHAR_EMPTY);
    agents[index][0] = location[0];
    agents[index][1] = location[1];
    grid[location[0]][location[1]] = grid[row][col];
    grid[row][col] = CHAR_EMPTY;

    printf("\nThe agent in (%d,%d) has been moved on (%d,%d)", row, col, location[0], location[1]);
}