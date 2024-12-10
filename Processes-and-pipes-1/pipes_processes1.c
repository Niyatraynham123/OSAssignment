#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<string.h>
#include<sys/wait.h>

int main()
{
    // We use two pipes:
    // - First pipe to send input string from parent (P1) to child (P2)
    // - Second pipe to send the second string from child (P2) back to parent (P1)

    int fd1[2];  // First pipe: Parent to child
    int fd2[2];  // Second pipe: Child to parent

    char fixed_str[] = "howard.edu";
    char input_str[100];
    char second_input_str[100];
    pid_t p;

    if (pipe(fd1) == -1)
    {
        fprintf(stderr, "Pipe Failed\n");
        return 1;
    }

    if (pipe(fd2) == -1)
    {
        fprintf(stderr, "Pipe Failed\n");
        return 1;
    }

    // Parent (P1) enters a string
    printf("Enter a string to concatenate: ");
    scanf("%s", input_str);

    p = fork();

    if (p < 0)
    {
        fprintf(stderr, "Fork Failed\n");
        return 1;
    }

    // Parent process (P1)
    else if (p > 0)
    {
        close(fd1[0]);  // Close the read end of fd1
        close(fd2[0]);  // Close the read end of fd2

        // Write input string to the first pipe and close the write end
        write(fd1[1], input_str, strlen(input_str) + 1);
        close(fd1[1]);

        // Wait for the child to finish and send a string back
        wait(NULL);

        // Read the concatenated string from the second pipe
        close(fd2[1]);  // Close the write end of fd2
        read(fd2[0], second_input_str, 100);
        close(fd2[0]);

        // Concatenate "gobison.org" and print the final result
        strcat(second_input_str, "gobison.org");
        printf("Final string: %s\n", second_input_str);
    }

    // Child process (P2)
    else
    {
        close(fd1[1]);  // Close the write end of fd1
        close(fd2[1]);  // Close the write end of fd2

        // Read the string from the first pipe
        char concat_str[100];
        read(fd1[0], concat_str, 100);
        close(fd1[0]);

        // Concatenate "howard.edu"
        strcat(concat_str, "howard.edu");
        printf("Concatenated string: %s\n", concat_str);

        // Prompt for a second string and send it to the parent
        printf("Enter a second string: ");
        scanf("%s", second_input_str);

        // Send the second input back to the parent process
        write(fd2[1], second_input_str, strlen(second_input_str) + 1);
        close(fd2[1]);

        exit(0);
    }

    return 0;
}
