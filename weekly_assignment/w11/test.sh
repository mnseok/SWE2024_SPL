#!/bin/bash

# Install bc if it is not installed
if ! [ -x "$(command -v bc)" ]; then
    echo "Warning: bc is not installed. Install bc by running 'sudo apt-get install bc'." >&2
    sudo apt-get install bc -y
fi

num_cores=$(nproc)
if [ $num_cores -lt 4 ]; then
    echo "Warning: Your machine has less than 4 cores. The speedup may not be accurate." >&2
fi

# Do not use math.h
cat monte_carlo_multi_thread.c | grep -q "#include <math.h>" && echo "Error: Do not use math.h" && exit 1

make single
make multi

check() {
    if [ "$1" != "PI = 3.14" ]; then
        echo "Error: the output was $1"
        make clean
        exit 1
    fi
}

TRIALS=100

# Run ./single TRIALS times and print the average time in milliseconds
echo "Single Thread"
sum1=0
for ((i = 1; i <= TRIALS; i++))
do
    start=$(date +%s.%N)
    if [ $num_cores -lt 4 ]; then
        output=$(./single)
    else
        output=$(taskset -c 0-3 ./single)
    fi
    end=$(date +%s.%N)

    check "$output"

    sum1=$(bc -l <<< "$sum1 + ($end - $start)")
done
echo "Average time: $(bc -l <<< "scale=3; $sum1 * 100 / 10") ms"

# Run ./multi 100 times and print the average time in milliseconds
echo "Multi Thread"
sum2=0
for ((i = 1; i <= TRIALS; i++))
do
    start=$(date +%s.%N)
    if [ $num_cores -lt 4 ]; then
        output=$(./multi)
    else
        output=$(taskset -c 0-3 ./multi)
    fi
    end=$(date +%s.%N)

    check "$output"

    sum2=$(bc -l <<< "$sum2 + ($end - $start)")
done
echo "Average time: $(bc -l <<< "scale=3; $sum2 * 100 / 10") ms"

echo "Speedup: $(bc -l <<< "scale=2; $sum1 / $sum2 * 100")%"

make clean
