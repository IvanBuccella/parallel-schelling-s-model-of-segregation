#!/bin/bash
cd /app

echo "Compiling the project"
mpicc main.c -o main.out
echo "End compiling"

echo -e "\n------------------Starting the test------------------"

if test -f "x-processors.txt"; then
    rm x-processors.txt
fi
if test -f "two-processors.txt"; then
    rm two-processors.txt
fi

echo -e "\n\nRun with 2 processors"
mpirun -np 2 --allow-run-as-root ./main.out >> two-processors.txt
for i in {3..16}
do
  echo -e "\n\nRun with $i processors"
  mpirun -np $i --allow-run-as-root ./main.out >> x-processors.txt
  if ! cmp -s two-processors.txt x-processors.txt; then
    echo -e "Differences found."
    exit 1
  else
    echo -e "No differences found."
  fi
  if test -f "x-processors.txt"; then
      rm x-processors.txt
  fi  
done
echo -e "\n------------------Test ended------------------"