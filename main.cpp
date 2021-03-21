#include <iostream>
#include <cstring>
#include <string>
#include <dirent.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/errno.h>
#include <fstream>
#include <fcntl.h>
#include <signal.h>
#include "signals_parent.h"
using namespace std;
int* workers_pid_array;
int numWorkers;
int* readfd;
int* writefd;
char** fifo1;
char** fifo2;
char* bufferSize;
int directories_per_worker;
char ***workers_directory_array;
int successes, fails;
char* input_dir;
string input;

void sig_handler(int signo) { // When /exit is given, restore default (ignore)
	cout << "parent signal handler" << endl;
	if (signo == SIGCHLD) {
		// sleep(1);
		parent_replace_kid(workers_pid_array, numWorkers, writefd, readfd, fifo1, fifo2, bufferSize, directories_per_worker, workers_directory_array);
	}
	else if (signo == SIGUSR1) { // When a worker gets new files in its directory, the parent has to read the summary.
		cout << "Parent received SIGUSR1 to read new summary" << endl;
		receive_summary_from_new_files(numWorkers, writefd, readfd, bufferSize);
	}
	else if (signo == SIGINT || signo == SIGQUIT) {
		signal(SIGCHLD, SIG_IGN); // Don't replace workers!
		kill_workers_and_terminate(numWorkers, workers_pid_array, writefd, readfd, fifo1, fifo2, bufferSize, workers_directory_array, directories_per_worker, successes, fails, input_dir, input);
	}
}

int main(int argc, char const *argv[]) {
	signal(SIGUSR1, sig_handler);
	signal(SIGCHLD, sig_handler);
	signal(SIGINT, sig_handler);
	signal(SIGQUIT, sig_handler);
	sigset_t mask1;
	sigemptyset(&mask1);
	sigaddset(&mask1, SIGINT);
	sigaddset(&mask1, SIGQUIT);
	sigaddset(&mask1, SIGCHLD);
	sigaddset(&mask1, SIGUSR1);
	sigprocmask(SIG_BLOCK, &mask1, NULL); // Block these signals while setting up workers

	// cout << "Parent PID: " << getpid() << endl;
	// cout << "Parent's parent PID: " << getppid() << endl;
	string patientRecordsFile = "";
	numWorkers = -1;
	int input_dir_index = -1;
	successes = 0;
	fails = 0;
	// string input_dir = "";
	bufferSize = new char[5];
	strcpy(bufferSize, "1234");
	for (int i = 1; i < argc-1; i=i+2) {
		if (strcmp(argv[i],"-w") == 0) {
			for (int j = 0; j < (int) strlen(argv[i+1]); ++j) {
				if (argv[i+1][j] > '9' || argv[i+1][j] < '0') {
					cout << "Number of workers is not a number. Terminating." << endl;
					return 1;
				}
			}
			numWorkers = atoi(argv[i+1]); // CHECK IF THIS IS A NUMBER
		}
		else if (strcmp(argv[i],"-b") == 0)
			strcpy(bufferSize, argv[i+1]);
		else if (strcmp(argv[i],"-i") == 0) {
			// input_dir = argv[i+1];
			input_dir_index = i+1;
		}
		else
			cout << "MISTAKE" << endl;
	}
	if (numWorkers <= 0 || strcmp(bufferSize, "1234") == 0 || input_dir_index == -1) {
		cout << "Could not find expected arguments. Terminating." << endl;
		return 1;
	}
	// cout << numWorkers << "  " << bufferSize << "  " << argv[input_dir_index] << endl;
	int input_dir_length = strlen(argv[input_dir_index]);
	// char* input_dir;
	if (argv[input_dir_index][input_dir_length - 1] != '/') { // If there's not a / at the end, add it now
		input_dir = new char[input_dir_length + 2];
		strcpy(input_dir, argv[input_dir_index]);
		input_dir[input_dir_length] = '/';
		input_dir[input_dir_length + 1] = '\0';
	}
	else {
		input_dir = new char[input_dir_length + 1];
		strcpy(input_dir, argv[input_dir_index]);
	}

	// Create directories array to send to workers

	int directory_count = 0;
	DIR * dirp;
	struct dirent * entry;
	dirp = opendir(input_dir); // I took 2 lines from stackoverflow to get me started with the directory
	if (dirp == NULL) {
		cout << "Could not open specified directory. Terminating." << endl;
		return 1;
	}
	int longest_directory_name = 0;
	while ((entry = readdir(dirp)) != NULL) {
		if (entry->d_type == DT_DIR && ((entry->d_name[0] >= 65 && entry->d_name[0] <= 91) || (entry->d_name[0] >= 97 && entry->d_name[0] <= 122) || (entry->d_name[0] >= 48 && entry->d_name[0] <= 57))) { // If the entry is a directory and not . or ..
			directory_count++; // This is to set an appropriate size for an array below
			if ( (int) strlen(entry->d_name) > longest_directory_name)
				longest_directory_name = strlen(entry->d_name); // This is also to set an appropriate size for an array below
		}
	}
	closedir(dirp);
	if (directory_count % numWorkers == 0)
		directories_per_worker = directory_count / numWorkers;
	else
		directories_per_worker = directory_count / numWorkers + 1;

	workers_directory_array = new char**[numWorkers];
	for (int i = 0; i < numWorkers; ++i) {
		workers_directory_array[i] = new char*[directories_per_worker + 1];
		for (int j = 0; j < directories_per_worker; ++j) {
			workers_directory_array[i][j] = new char[strlen(input_dir) + longest_directory_name + 3];
		}
	}
	// cout << "length: " << strlen(argv[input_dir_index]) + longest_directory_name + 1 << endl;
	for (int i = 0; i < numWorkers; ++i) {
		for (int j = 0; j < directories_per_worker; ++j) {
			// workers_directory_array[i][j][0] = '\0'; // This is to make sure 
			strcpy(workers_directory_array[i][j], "(none)");
		}
		workers_directory_array[i][directories_per_worker] = NULL; // NULL as n+1 delimiter for execv()
	}
	// cout << "Array size: [" << numWorkers << "][" << directories_per_worker << "]" << endl;

	int directory_i = -1;
	dirp = opendir(input_dir);
	if (dirp == NULL) {
		cout << "Could not open specified directory. Terminating." << endl;
		return 1;
	}
	while ((entry = readdir(dirp)) != NULL) {
		if (entry->d_type == DT_DIR && ((entry->d_name[0] >= 65 && entry->d_name[0] <= 91) || (entry->d_name[0] >= 97 && entry->d_name[0] <= 122) || (entry->d_name[0] >= 48 && entry->d_name[0] <= 57))) { // If the entry is a directory and not . or ..
			directory_i++;
			// cout << "directory_count: " << directory_i << endl;
			// cout << "Copying " << entry->d_name << " in [" << directory_i % numWorkers << "][" << directory_i / numWorkers << "]" << endl;
			strcpy(workers_directory_array[directory_i % numWorkers][directory_i / numWorkers], input_dir);
			strcat(workers_directory_array[directory_i % numWorkers][directory_i / numWorkers], entry->d_name);
			strcat(workers_directory_array[directory_i % numWorkers][directory_i / numWorkers], "/");
		}
	}
	closedir(dirp);
	// for (int i = 0; i < numWorkers; ++i) {
	// 	for (int j = 0; j < directories_per_worker; ++j) {
	// 		cout << "[" << i << "][" << j << "] - " << workers_directory_array[i][j] << endl;
	// 	}
	// }
	// cout << "Parent: about to make fifos" << endl;

	// int readfd, writefd;
	char temp10[10];
	readfd = new int[numWorkers];
	writefd = new int[numWorkers];
	fifo1 = new char*[numWorkers];
	fifo2 = new char*[numWorkers];
	for (int i = 0; i < numWorkers; ++i) {
		fifo1[i] = new char[15];
		fifo2[i] = new char[15];
	}
	for (int i = 0; i < numWorkers; i++) {
		readfd[i] = 2*i;
		writefd[i] = 2*i+1;
		strcpy(fifo1[i], "fifo.");
		sprintf(temp10, "%d", 2*i);
		strcat(fifo1[i], temp10);
		strcpy(fifo2[i], "fifo.");
		sprintf(temp10, "%d", 2*i+1);
		strcat(fifo2[i], temp10);

		if (mkfifo(fifo1[i], 0666) < 0 && (errno != EEXIST)) {
			perror("Cannot create FIFO");
		}
		if (mkfifo(fifo2[i], 0666) < 0 && (errno != EEXIST)) {
			unlink(fifo2[i]);
			perror("Cannot create FIFO");
		}
	}

	workers_pid_array = new int[numWorkers];
	for (int i = 0; i < numWorkers; ++i) {
		int fork_return = fork();
		// cout << "fork return: " << fork_return << endl;
		if (fork_return == 0) // Only the KIDD should exec this, not the parent.
			execl("./worker", "./worker", fifo1[i], fifo2[i], bufferSize, "y", NULL); // For some reason, valgrind replaces the first argument
			// ...with the executable name, while, without valgrind, worker's argv[0] is the first argument
		// cout << "Parent: about to open fifos" << endl;
		if ((readfd[i] = open(fifo1[i], 0)) < 0) {
			perror("Parent: cannot open read FIFO");
		}
		// cout << "Opened one\n";
		if ((writefd[i] = open(fifo2[i], 1)) < 0) {
			perror("Parent: cannot open write FIFO");
		}
		// cout << "Parent: opening done" << endl;
		workers_pid_array[i] = fork_return;
	}

	char buff[100] = "-nothing-";
	for (int i = 0; i < numWorkers; ++i) {
		sprintf(buff, "%d", directories_per_worker);
		buff[3] = '\0';
		write(writefd[i], buff, 4); // SEND WORKER NUMBER OF DIRECTORIES
		read(readfd[i], buff, 4); // Receive 'ok number of directories'
		for (int j = 0; j < directories_per_worker; ++j) {
			if (workers_directory_array[i][j] != NULL) {
				char ok_response[10];
				char answer_size_array[10];
				strcpy(answer_size_array, "_________");
				int answer_size = strlen(workers_directory_array[i][j]);
				sprintf(answer_size_array, "%d", answer_size);
				write(writefd[i], answer_size_array, 10); // SEND RESPONSE SIZE
				read(readfd[i], ok_response, 6); // READ 'RESPONSE SIZE RECEIVED OK'
				int send_index = 0;
				int send_size = atoi(bufferSize);
				do { // In every loop, send bufferSize bytes, or however many we need to complete the message. Other side knows to read answer_size bytes
					if (send_index + send_size > answer_size + 1)
						send_size = answer_size + 1 - send_index;
					write(writefd[i], workers_directory_array[i][j] + send_index, send_size);
					send_index += send_size;
					read(readfd[i], ok_response, 6);
				} while (send_index < answer_size + 1);
				write(writefd[i], ok_response, 6);
			}
		}
	}
	// cout << "DONE SENDING DIRECTORIES" << endl;
	// Now, receive summary statistics.
	char ok_response[10];
	char transfer_size[15];
	for (int i = 0; i < numWorkers; ++i) {
		do { // READ SUMMARY FROM ALL FILES OF ONE WORKER
			// cout << "PARENT LOOP" << endl;
			read(readfd[i], transfer_size, 10); // READ RESPONSE SIZE
			if (strncmp(transfer_size, "#end#", 5) == 0)
				break; // NO MORE SUMMARY TO READ
			// cout << "Parent: 'response size' received: " << transfer_size << endl;
			strcpy(ok_response, "OKres");
			write(writefd[i], ok_response, 6); // SEND 'RESPONSE SIZE OK'
			char response[atoi(transfer_size)];
			int response_size_int = atoi(transfer_size);

			// read(readfd, response, response_size_int + 1); // READ RESPONSE
			int write_index = 0;
			int receive_size = atoi(bufferSize);
			do {
				if (write_index + receive_size > response_size_int + 1)
					receive_size = response_size_int + 1 - write_index;
				read(readfd[i], response + write_index, receive_size);
				write_index += receive_size;
				response[write_index] = '\0';
				// cout << "Received so far: " << response << endl;
				write(writefd[i], ok_response, 6);
			} while (write_index < response_size_int + 1);
			read(readfd[i], ok_response, 6);
			cout << response << endl;
		} while (/*strncmp(ok_response, "#end#", 5) != 0*/1);
		// cout << "DONE READING SUMMARIES" << endl;
	}

	// WE PUT +1 FOR THE \0 IN ALL 'READ' AND 'WRITE' FUNCTIONS
	// IF AN ERROR MESSAGE IS SENT FROM WORKER INSTEAD OF AN ANSWER, OR IT DIDN'T FIND WHAT WE ASKED, IT STARTS WITH !,
	//        SO WE CAN CHECK THE 1ST CHARACTER AND FIND OUT
	int quitt = 0;
	string input_tokens[8];
	while (quitt == 0) { // READ USER INPUT AND SEND TO WORKER
		sigprocmask(SIG_UNBLOCK, &mask1, NULL); // Now that workers are ready, accept signals
		cout << endl << "Type a command (or send a signal): ";
		getline(cin, input);
		if (input == "")
			input = "(none)";
		char char_line[input.length() + 1];        // Char array
		for (int i = 0; i < (int)input.length(); ++i) { // Copy string to char array
			char_line[i] = input[i];
		}
		char_line[input.length()] = '\0';          // Make sure it ends with a \0
		// Block signals while executing user input
		sigprocmask(SIG_BLOCK, &mask1, NULL);
		// cout << "     You typed: " << char_line << endl;
		if (strncmp(char_line, "/listCountries", 15) == 0) { // This query is answered by the parent only
			for (int i = 0; i < numWorkers; ++i) {
				for (int j = 0; j < directories_per_worker; ++j) {
					if (strncmp(workers_directory_array[i][j], "(none)", 6) == 0) // Some workers have one directory less
						continue;
					// Extract country from directory and print it along with i
					int country_letter_index = strlen(workers_directory_array[i][j]) - 2;
					int country_length = 0;
					while (workers_directory_array[i][j][country_letter_index] != '/') {
						country_length++;
						country_letter_index--;
					}
					int country_start_index = strlen(workers_directory_array[i][j]) - country_length;
					string country;
					for (int k = country_start_index - 1; k < country_start_index + country_length - 1; ++k) {
						country = country + workers_directory_array[i][j][k];
					}
					cout << country << " " << workers_pid_array[i] << endl;
				}
			}
			continue;
		}
		else if (strncmp(char_line, "/exit", 6) == 0)
			signal(SIGCHLD, SIG_IGN); // If we're about to tell the workers to quit, don't make new ones!
		for (int i = 0; i < numWorkers; ++i) {
			///////////////////////////////////////////////////////////////////////////// Send to worker code
			char answer_size_array[10];
			strcpy(answer_size_array, "_________");
			int answer_size = strlen(char_line);
			sprintf(answer_size_array, "%d", answer_size);
			write(writefd[i], answer_size_array, 10); // SEND TRANSMISSION SIZE
			read(readfd[i], ok_response, 6); // READ 'RESPONSE SIZE RECEIVED OK'
			// write(writefd, answer_array, strlen(answer_array) + 1); // SEND ACTUAL RESPONSE
			int send_index = 0;
			int send_size = atoi(bufferSize);
			do {
				if (send_index + send_size > answer_size + 1)
					send_size = answer_size + 1 - send_index;
				// cout << "sending: " << send_size << endl;
				write(writefd[i], char_line + send_index, send_size);
				send_index += send_size;
				read(readfd[i], ok_response, 6);
			} while (send_index < answer_size + 1);
			write(writefd[i], ok_response, 6);
			///////////////////////////////////////////////////////////////////////////// End of send to worker code
		}
		int printed_an_answer = 0;
		int total_case2_cases = 0;
		for (int i = 0; i < numWorkers; ++i) { // READ RESPONSE FROM EVERY WORKER
			read(readfd[i], transfer_size, 10); // READ RESPONSE SIZE
			// cout << "Parent: 'response size' received: " << transfer_size << endl;
			strcpy(ok_response, "OKres");
			write(writefd[i], ok_response, 6); // SEND 'RESPONSE SIZE OK'
			char response[atoi(transfer_size)];
			int response_size_int = atoi(transfer_size);

			// read(readfd, response, response_size_int + 1); // READ RESPONSE
			int write_index = 0;
			int receive_size = atoi(bufferSize);
			do {
				if (write_index + receive_size > response_size_int + 1)
					receive_size = response_size_int + 1 - write_index;
				read(readfd[i], response + write_index, receive_size);
				write_index += receive_size;
				response[write_index] = '\0';
				// cout << "Received so far: " << response << endl;
				write(writefd[i], ok_response, 6);
			} while (write_index < response_size_int + 1);
			read(readfd[i], ok_response, 6);
			///////////////////////////////////////////////////

			if (strncmp(char_line, "/diseaseFrequency", 17) == 0 && response[0] != '!') {
				if (response[0] >= '0' && response[0] <= '9') {
					total_case2_cases += atoi(response);
				}
				if (i == numWorkers - 1) {
					cout << "WORKER RESPONSE:" << endl << total_case2_cases << endl;
					successes++;
				}
			}
			else if (strncmp(response, "exiting", 7) == 0) {
				signal(SIGCHLD, SIG_IGN);
				quitt = 1;
				// cout << "Parent received -exiting-." << endl;
				cout << "Worker " << i << " exiting" << endl;
				int exit_code = 0;
				waitpid(workers_pid_array[i], &exit_code, 0);
				exit_code = exit_code >> 8;
				// cout << "Exit code of " << i << ": " << exit_code << endl;
			}
			else if (response[0] != '!' || (i == numWorkers - 1 && printed_an_answer == 0)) { // If some kind of error happened, don't print it for every worker.
				if (response[0] == '!') {
					cout << "WORKER RESPONSE:" << endl << response + 1 << endl;
					fails++;
				}
				else {
					cout << "WORKER RESPONSE:" << endl << response << endl;
					successes++;
				}
				printed_an_answer = 1;
			}
		}
		// cout << "Waiting for signal for 5 seconds" << endl;
		// sleep(5);
	}
	signal(SIGCHLD, SIG_IGN);
	sigprocmask(SIG_UNBLOCK, &mask1, NULL);

	for (int i = 0; i < numWorkers; ++i) {
		for (int j = 0; j < directories_per_worker + 1; ++j) {
			delete[] workers_directory_array[i][j];
		}
		delete[] workers_directory_array[i];
	}
	delete[] workers_directory_array;

	delete[] workers_pid_array;
	for (int i = 0; i < numWorkers; ++i) {
		close(readfd[i]);
		close(writefd[i]);
	}
	delete[] writefd;
	delete[] readfd;
	for (int i = 0; i < numWorkers; ++i) {
		unlink(fifo1[i]);
		unlink(fifo2[i]);
		delete[] fifo1[i];
		delete[] fifo2[i];
	}
	delete[] fifo1;
	delete[] fifo2;
	delete[] bufferSize;
	delete[] input_dir;
	// exit(0);
	return 0;
}