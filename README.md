# A parallel algorithm for Schelling's model of segregation

## Introduction

This algorithm uses a parallel programming approach, by exploiting distributed memory, for solving Schelling's model of segregation. Its behaviour allows to satisfy all the agents into the grid in less time than usual by exploiting the concurrent execution over more processors or nodes.

## The problem

In 1971, the American economist Thomas Schelling created an agent-based model that suggested inadvertent behavior might also contribute to segregation. His model of segregation showed that even when individuals (or "agents") didn't mind being surrounded or living by agents of a different race or economic background, they would still choose to segregate themselves from other agents over time! Although the model is quite simple, it provides a fascinating look at how individuals might self-segregate, even when they have no explicit desire to do so.

The problem's to create a simulation of Schelling's model by using a parallel programming approach, by exploiting distributed memory.

### Definitions

There are two types of agents: (`B`)lue and (`R`)ed; and a cell can be populated or can be (`E`)mpty.

A `satisfied agent` is one that is surrounded by at least threshold `t` percent (30%) of agents that are like itself.

An `unsatisfied agent` is randomly moved `only` on a vacant location in the grid `owned by the corresponding processor`. This means if the grid `size`x`size` is divided by rows between 4 processors, when the agent in the cell [0,0] can be moved only in an empty cell in the first `size/4` rows.

The simulation is performed until `all agents are satisfied` or a maximum number of rounds `max_rounds` is reached.

The agents are initially placed into random locations of a neighborhood represented by a grid by assigning to the agent `x` the value = `{ B | (x % 2) == 0} ∪ { G | (x % 2) == 1}`.

The `MASTER` is the processor with the rank `0` and a generic `SLAVE x` has the rank = `{ p | p > 0 and p <= workers }` where `workers = P - 1` and `P` is the number of processors exploited.

### The solution

The solution follows these steps:

1.  The `MASTER` processor allocates the grid and place all the agents by using the `initialize_grid` and `initialize_agents` functions.
2.  Until the max_rounds number is `NOT` reached `OR` all agents are `NOT` satisfied:

    1. The `MASTER` processor splits the grid rows over the `SLAVE` processors and `send` the corresponding portion to them. <small>The grid is split by assigning `(size/workers)` rows to every `SLAVE` (except for the remaining rows that are assigned to the last `SLAVE` if the `size` is not divisible for `workers`)</small>.
    2. The `SLAVE` processor `receive` its protion of the matrix and move the unsatisfied agents by using the `optimize_agents` function. Then it `send` back to the `MASTER` processor its modified matrix.
    3. The `MASTER` processor `receives` from all the `SLAVE` processors their portions and updates its own matrix.

3.  The `MASTER` prints out the result of the simulation.

## Implementation details

## Execution Tutorial

This tutorial shows how to locally deploy and run the algorithm.

- **[Prerequisites](#prerequisites)**
- **[Repository](#repository)**
- **[Build](#build)**
- **[Test the correctness](#test-the-correctness)**
- **[Test the algorithm](#test-the-algorith)**

### Prerequisites

- [Docker and Docker Compose](https://www.docker.com) (Application containers engine)

### Repository

Clone the repository:

```sh
$ git clone https://github.com/IvanBuccella/parallel-schelling-s-model-of-segregation
```

### Build

Build the local environment with Docker:

```sh
$ docker-compose build
```

### Test the correctness

Test the correctness using the local environment with Docker:

```sh
$ docker-compose up correctness
```

### Test the algorithm

Test the algorithm with `strong scalability` using the local environment with Docker:

```sh
$ docker-compose up runner-strong
```

Test the algorithm with `weak scalability` using the local environment with Docker:

```sh
$ docker-compose up runner-weak
```

## The correctness

The correctness container executes the algorithm several times, from using `2` processors to `24` processors. It saves the output of the `2` processor execution in the file `two-processors.txt` that is compared to the file `x-processors.txt` (which corresponds to the output of the x processor `{ x-processor | x > 2 }`) in order to find differences in the results.

## Benchmarking

The benchmark tests, for weak and strong scalability, have been executed over 6 instances with 4 cores (e2-standard-4) using the [Google Cloud Plaform](https://cloud.google.com/gcp/).

### Strong scalability

Has been used a fixed matrix size of `2700 x 2700`; the results are presented below.

<small>Data reported are the average of three runs for every processor number change.</small>

| Workers number | Speed-up | Execution time |
| -------------- | -------- | -------------- |
| 1              | X        | Xs             |
| 2              | X        | Xs             |
| 3              | X        | Xs             |
| 4              | X        | Xs             |
| 5              | X        | Xs             |
| 6              | X        | Xs             |
| 7              | X        | Xs             |
| 8              | X        | Xs             |
| 9              | X        | Xs             |
| 10             | X        | Xs             |
| 11             | X        | Xs             |
| 12             | X        | Xs             |
| 13             | X        | Xs             |
| 14             | X        | Xs             |
| 15             | X        | Xs             |
| 16             | X        | Xs             |
| 17             | X        | Xs             |
| 18             | X        | Xs             |
| 19             | X        | Xs             |
| 20             | X        | Xs             |
| 21             | X        | Xs             |
| 22             | X        | Xs             |
| 23             | X        | Xs             |

### Weak scalability

Has been used a dynamic matrix size starting from `500 x 500` to `2700 x 2700`, by augmenting the matrix size of `100 x 100` for every new processor added; the results are presented below.

<small>Data reported are the average of three runs for every processor number change.</small>

| Workers number | Input size | Execution time |
| -------------- | ---------- | -------------- |
| 1              | 500        | Xs             |
| 2              | 600        | Xs             |
| 3              | 700        | Xs             |
| 4              | 800        | Xs             |
| 5              | 900        | Xs             |
| 6              | 1000       | Xs             |
| 7              | 1100       | Xs             |
| 8              | 1200       | Xs             |
| 9              | 1300       | Xs             |
| 10             | 1400       | Xs             |
| 11             | 1500       | Xs             |
| 12             | 1600       | Xs             |
| 13             | 1700       | Xs             |
| 14             | 1800       | Xs             |
| 15             | 1900       | Xs             |
| 16             | 2000       | Xs             |
| 17             | 2100       | Xs             |
| 18             | 2200       | Xs             |
| 19             | 2300       | Xs             |
| 20             | 2400       | Xs             |
| 21             | 2500       | Xs             |
| 22             | 2600       | Xs             |
| 23             | 2700       | Xs             |

## Conclusions

## Contributing

This project welcomes contributions and suggestions. If you use this code, please cite this repository.

## Citation

Credit to [Carmine Spagnuolo](https://spagnuolocarmine.github.io/): [Schelling's model of segregation](https://spagnuolocarmine.notion.site/3-Schelling-s-model-of-segregation-272acd31bdcc45689abeb18593ed0dda) & [Ubuntu with OpenMPI and OpenMP](https://github.com/spagnuolocarmine/ubuntu-openmpi-openmp) & [docker-mpi ](https://github.com/spagnuolocarmine/docker-mpi).

Credit to Frank McCown: [Schelling's Model of Segregation](http://nifty.stanford.edu/2014/mccown-schelling-model-segregation/)
