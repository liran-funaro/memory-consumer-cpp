# Memory Consumer (C++)

MemoryConsumer is a program which its performance depends on the
total memory and the load of the program.

The program is constantly controlling an array of 1MB chunks.
According to the current load, the program create threads which
constantly writing to the memory. This causes the performance to
increase as the load increases.
Writing to the memory is done by picking a random number between 0 and
`max-rand`, and if the number is with the 1MB object limit, the
program is writing the object to the memory and increasing a counter.
This causes the performance to increase as the available memory
increases up to the point it reaches `max-rand`.
Performance is calculated by the amount of MBs written divided by the elapsed time.

# Usage

```bash
./memory_consumer <int: max memory (MB)> <float: sleep after write (seconds)>
```

`max memory`: The maximal memory consumption. 
`sleep after write`: The amount of time in seconds that each memory-writing thread will sleep after writing 1MB of memory.

# Compile
```bash
make
```

# License
[GPL](LICENSE.txt)
