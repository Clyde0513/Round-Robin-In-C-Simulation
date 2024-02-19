# You Spin Me Round Robin

This algorithm implements a simulation of round robin in C, given the quantum length and processes,
as well as its PIDs, arrival time, and burst times, respectively.

## Building

```shell
make
```

## Running

cmd for running

```shell
./rr processes.txt n
```

where n is the number of quantum length. The first line in the txt processes is the number of
processes in the simulation system. Then in each line, the first column is PID, then 2nd column is
arrival time, and the last column is burst time.
results

```
Results:

for 3 quantum length (same txt file):
Average waiting time: 7.00
Average response time: 2.75
for 9 quantum length (same txt file):
Average waiting time: 4.75
Average response time: 4.75
for 5 quantum length (same txt file):
Average waiting time: 5.50
Average response time: 3.25
```

## Cleaning up

```shell
make clean
```
