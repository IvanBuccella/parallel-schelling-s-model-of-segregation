#!/bin/bash
cd /app

echo "Compiling the project"
mpicc main.c -o main.out
echo "End compiling"

echo "Starting the test"
echo "Start test" > log.txt
for i in {2..4}
do
  echo "Run with $i processors" >> log.txt
  mpirun -np $i --allow-run-as-root ./main.out >> log.txt
done
echo "Test ended"