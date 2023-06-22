#!/bin/bash
cd /app

echo "Compiling the project"
mpicc main.c -o main.out
echo "End compiling"

echo -e "\n------------------Starting the test------------------"
for i in {2..16}
do
  echo -e "\n\nRun with $i processors"
  mpirun -np $i --allow-run-as-root ./main.out 2700
done
echo -e "\n------------------Test ended------------------"