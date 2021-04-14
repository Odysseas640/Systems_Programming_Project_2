# Systems_Programming_Project_2
Contains multiple processes, pipes, signals and a bash script. Built on top of the 1st assignment.

There is a main "parent" process, and multiple "worker" processes are its children. Each worker starts by reading his own input files and storing that data in RAM.
After the workers have finished, the parent receives queries from the user about the workers' data. The parent forwards the queries to the relevant workers through named pipes, they answer, and the parent prints the results.
If a worker terminates unexpectedly, the parent has to create a new one and have it read the same data as the one it's replacing.
If SIGUSR1 is sent to a worker, he has to look through his input directory to see if there are new files, and if there are, read them and store the data in RAM.
