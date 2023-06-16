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

#define size 2000      // The grid size - remember that the maximum integer value is 2147483647
#define empty_perc 30  // The percentage threshold of empty cells
#define t 30           // The percentage threshold of agent satisfaction
#define max_rounds 500 // The max rounds number for satisfing the agents
#define num_agents ((size * size) * (100 - empty_perc)) / 100

#define CHAR_BLUE 'B'
#define CHAR_RED 'R'
#define CHAR_EMPTY '-'

static char grid[size][size];
static int agents[num_agents][2];

void print_grid();
void initialize_agents();
bool is_satisfied(int index);
void move_agent(int index);
bool all_agents_are_satisfied();

int main()
{
    clock_t time;

    printf("The grid size is: %dx%d", size, size);
    printf("\nThe percentage threshold of empty cells is: %d", empty_perc);
    printf("\nThe percentage threshold of agent satisfaction is: %d", t);
    printf("\nThe max rounds number is: %d", max_rounds);
    printf("\nThe number of agents is: %d", num_agents);

    time = clock();

    initialize_agents();
    print_grid();

    int i = 0, unsatisfied_agents = 0, round = 0;
    while (i < num_agents && round < max_rounds)
    {
        if (i == 0)
        {
            printf("\n\nRound #%d started.", round);
        }
        if (!is_satisfied(i))
        {
            unsatisfied_agents++;
            move_agent(i);
        }
        i++;
        if (i == num_agents - 1)
        {
            printf("\nRound #%d ended.", round);
            round++;
            if (unsatisfied_agents > 0 && round < max_rounds - 1)
            {
                i = 0;
                unsatisfied_agents = 0;
            }
        }
    }

    print_grid();

    if (all_agents_are_satisfied())
    {
        printf("\nAll agents are satisfied");
    }
    else
    {
        printf("\nAll agents are not satisfied");
    }

    time = clock() - time;
    printf("\nTime in second = %f\n", ((double)time) / CLOCKS_PER_SEC);

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
    printf("\n\n");
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
    if ((siblings * 100) / neighbors >= t)
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

    printf("\nThe agent in (%d,%d) is not satisfied and has been moved on (%d,%d)", row, col, location[0], location[1]);
}

bool all_agents_are_satisfied()
{
    int i;
    for (i = 0; i < num_agents; i++)
    {
        if (!is_satisfied(i))
        {
            return false;
        }
    }
    return true;
}