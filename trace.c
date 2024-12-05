#include "types.h"
#include "user.h"

int main() {
    int pid = fork(); // Create a child process

    if (pid < 0) {
        // Fork failed
        printf(2, "Fork failed\n");
        exit();
    } else if (pid == 0) {
        // Child process code
        printf(1, "Child process: PID = %d\n", getpid());
        printf(1, "Child says: Hello, world!\n");
        exit(); // Exit child process
    } else {
        // Parent process code
        printf(1, "Parent process: PID = %d, Child PID = %d\n", getpid(), pid);
        wait(); // Wait for the child process to finish
        printf(1, "Parent says: Goodbye!\n");
    }

    exit(); // Exit parent process
}
