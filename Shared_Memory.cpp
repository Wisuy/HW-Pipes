#include <iostream>
#include <vector>
#include <unistd.h>
#include <sys/wait.h>

using namespace std;

// Function to check whether a number is prime
bool isPrime(int num) {
    if (num < 2) {
        return false;
    }

    for (int i = 2; i * i <= num; ++i) {
        if (num % i == 0) {
            return false;
        }
    }

    return true;
}

int main() {
    // Create an array of pipes for communication between processes
    const int numProcesses = 10;
    const int numbersPerProcess = 1000; // Divide the search for prime numbers into 10 segments
    int pipes[numProcesses][2];

    // Create a pipe for each process pair
    for (int i = 0; i < numProcesses; ++i) {
        if (pipe(pipes[i]) == -1) {
            cerr << "Error creating pipe " << i << endl;
            return 1;
        }
    }

    // Fork 10 child processes
    for (int i = 0; i < numProcesses; ++i) {
        pid_t pid = fork();

        if (pid == -1) {
            cerr << "Error forking process " << i << endl;
            return 1;
        }

        if (pid == 0) {
            // Close the read end of the pipe for the child process
            close(pipes[i][0]);

            // Assign the starting and ending indices for the search in this segment
            int start = i * numbersPerProcess + 1;
            int end = (i + 1) * numbersPerProcess;

            // Find prime numbers in the assigned segment and write them to the pipe
            for (int num = start; num <= end; ++num) {
                if (isPrime(num)) {
                    write(pipes[i][1], &num, sizeof(int));
                }
            }

            // Close the write end of the pipe for the child process
            close(pipes[i][1]);

            // Child process exits after writing its prime numbers
            exit(0);
        }
    }

    // Close the write ends of all pipes (not needed by the parent process)
    for (int i = 0; i < numProcesses; ++i) {
        close(pipes[i][1]);
    }

    // Display a heading for the prime numbers
    cout << "Prime numbers up to 10000:" << endl;

    // Read prime numbers from each pipe and display them
    for (int i = 0; i < numProcesses; ++i) {
        int primeNum;

        while (read(pipes[i][0], &primeNum, sizeof(int)) > 0) {
            cout << primeNum << " ";
        }

        // Close the read end of the pipe for the current iteration
        close(pipes[i][0]);
    }

    // Wait for all child processes to finish
    for (int i = 0; i < numProcesses; ++i) {
        wait(NULL);
    }

    cout << endl;

    return 0;
}
