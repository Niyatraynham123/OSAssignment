#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>

/**
 * Executes the command: "cat scores | grep <arg> | sort".
 * This program chains three processes together: cat -> grep -> sort.
 * The grep argument is passed as a command-line argument.
 */

int main(int argc, char **argv)
{
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <grep_argument>\n", argv[0]);
        return 1;
    }

    int pipefd1[2];  // Pipe 1: cat -> grep
    int pipefd2[2];  // Pipe 2: grep -> sort
    int pid1, pid2;

    // Prepare arguments
    char *cat_args[] = {"cat", "scores", NULL};
    char *grep_args[] = {"grep", argv[1], NULL};
    char *sort_args[] = {"sort", NULL};

    // Create two pipes
    if (pipe(pipefd1) == -1 || pipe(pipefd2) == -1) {
        perror("Pipe failed");
        return 1;
    }

    // First fork for grep
    pid1 = fork();
    if (pid1 == 0) {
        // Child P2: Execute grep

        // Close unused pipe ends
        close(pipefd1[1]);
        close(pipefd2[0]);

        // Redirect stdin to pipefd1
        dup2(pipefd1[0], 0);
        // Redirect stdout to pipefd2
        dup2(pipefd2[1], 1);

        // Close the pipes after redirecting
        close(pipefd1[0]);
        close(pipefd2[1]);

        // Execute grep
        execvp("grep", grep_args);
        perror("Execvp failed for grep");
        return 1;
    } 

    // Second fork for sort
    pid2 = fork();
    if (pid2 == 0) {
        // Child P3: Execute sort

        // Close unused pipe ends
        close(pipefd1[0]);
        close(pipefd1[1]);
        close(pipefd2[1]);

        // Redirect stdin to pipefd2
        dup2(pipefd2[0], 0);

        // Close the pipes after redirecting
        close(pipefd2[0]);

        // Execute sort
        execvp("sort", sort_args);
        perror("Execvp failed for sort");
        return 1;
    }

    // Parent P1: Execute cat

    // Close unused pipe ends
    close(pipefd1[0]);
    close(pipefd2[0]);
    close(pipefd2[1]);

    // Redirect stdout to pipefd1
    dup2(pipefd1[1], 1);

    // Close the pipes after redirecting
    close(pipefd1[1]);

    // Execute cat
    execvp("cat", cat_args);
    perror("Execvp failed for cat");
    return 1;
}

