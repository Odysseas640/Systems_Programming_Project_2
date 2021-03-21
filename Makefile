CC=g++
CFLAGS=-Wall -std=c++11 -ggdb3

all: date.o date_tree.o hash_table.o patientRecord.o lists.o signals_worker.o signals_parent.o main_worker.o worker diseaseAggregator

lists.o: lists.cpp lists.h
	$(CC) $(CFLAGS) -c lists.cpp

date.o: date.cpp date.h
	$(CC) $(CFLAGS) -c date.cpp

date_tree.o: date_tree.cpp date_tree.h
	$(CC) $(CFLAGS) -c date_tree.cpp

hash_table.o: hash_table.cpp hash_table.h
	$(CC) $(CFLAGS) -c hash_table.cpp

patientRecord.o: patientRecord.cpp patientRecord.h
	$(CC) $(CFLAGS) -c patientRecord.cpp

signals_worker.o: signals_worker.cpp signals_worker.h
	$(CC) $(CFLAGS) -c signals_worker.cpp

signals_parent.o: signals_parent.cpp signals_parent.h
	$(CC) $(CFLAGS) -c signals_parent.cpp

main_worker.o: main_worker.cpp
	$(CC) $(CFLAGS) -c main_worker.cpp

worker: main_worker.o lists.o signals_worker.o patientRecord.o hash_table.o date_tree.o date.o
	$(CC) $(CFLAGS) -o worker main_worker.o hash_table.o patientRecord.o date_tree.o date.o lists.o signals_worker.o

diseaseAggregator: main.cpp signals_parent.cpp
	$(CC) $(CFLAGS) -o diseaseAggregator main.cpp signals_parent.cpp

.PHONY: clean

clean:
	rm -f worker main_worker.o hash_table.o patientRecord.o date_tree.o date.o lists.o signals_worker.o signals_parent.o diseaseAggregator
