#!/bin/bash
cd /app

echo "Compiling the project"
mpicc main.c -o main.out
echo "End compiling"

size=500

echo -e "\n------------------Starting the test------------------"
for i in {2..24}
do
  echo -e "\n\nRun with $i processors"
  echo $size
  mpirun -np $i --allow-run-as-root ./main.out $size
  ((size=size+100))
done
echo -e "\n------------------Test ended------------------"