# A parallel algorithm for Schelling's model of segregation

## Introduction

This algorithm uses a parallel programming approach, by exploiting distributed memory, for solving Schelling's model of segregation. Its behaviour allows to satisfy all the agents into the grid in less time than usual by exploiting the concurrent execution over more processors or nodes.

## The problem

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

### Test the correctness locally

Test the correctness using the local environment with Docker:

```sh
$ docker-compose up correctness
```

### Test the algorithm locally

Test the algorithm using the local environment with Docker:

```sh
$ docker-compose up runner
```

## The correctness

The correctness container executes the algorithm several times, from using `2` processors to `10` processors. It saves the output of the `2` processor execution in the file `two-processors.txt` that is compared to the file `x-processors.txt` (which corresponds to the output of the x processor `{ x-processor | x > 2 }`) in order to find differences in the results.

## Benchmarking

### Architecture

The benchmark is exploited over 6 instances with 4 cores (e2-standard-4) using the [Google Cloud Plaform](https://cloud.google.com/gcp/).

### Strong Scaling

### Weak Scaling

## Conclusions
