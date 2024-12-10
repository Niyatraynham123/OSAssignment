#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <sys/wait.h>

// Function prototype for the client process
void ClientProcess(int sharedMem[]);

int main(int argc, char *argv[]) {
    int shmID;
    int *shmPTR;
    pid_t pid;
    int status;

    // Create shared memory segment for 2 integers
    shmID = shmget(IPC_PRIVATE, 2 * sizeof(int), IPC_CREAT | 0666);
    if (shmID < 0) {
        perror("*** shmget error (server) ***");
        exit(1);
    }
    printf("Server has received shared memory for 2 integers...\n");

    // Attach the shared memory segment
    shmPTR = (int *) shmat(shmID, NULL, 0);
    if ((long)shmPTR == -1) {
        perror("*** shmat error (server) ***");
        exit(1);
    }
    printf("Server has attached shared memory...\n");

    // Initialize shared memory values
    shmPTR[0] = 0;  // Account balance
    shmPTR[1] = 0;  // Control flag for synchronization

    // Fork a child process
    printf("Server is about to fork a child process...\n");
    pid = fork();
    if (pid < 0) {
        perror("*** fork error (server) ***");
        exit(1);
    } else if (pid == 0) {
        // Child process executes ClientProcess function
        ClientProcess(shmPTR);
        exit(0);
    }

    // Parent process continues execution
    srand(42);  // Seed the random number generator
    int i = 0;
    int account, balance;
    int sleep_time;

    // Parent process performs deposits
    while (waitpid(pid, &status, WNOHANG) == 0) {  // Child is still running
        while (i < 25) {
            sleep_time = rand() % 6;  // Random sleep time (0 - 5 seconds)
            sleep(sleep_time);

            // Wait until child has completed a transaction
            while (shmPTR[1] != 0);

            account = shmPTR[0];

            // Perform deposit if account balance is <= 100
            if (account <= 100) {
                balance = rand() % 101;  // Random deposit (0 - 100)

                if (balance % 2 == 0) {
                    account += balance;
                    printf("Dear old Dad: Deposits $%d / Balance = $%d\n", balance, account);
                } else {
                    printf("Dear old Dad: Doesn't have any money to give\n");
                }

                shmPTR[0] = account;
            }

            // Set flag to indicate parent transaction is done
            shmPTR[1] = 1;
            i++;
        }
    }

    // Parent has detected the completion of its child
    printf("Parent has detected the completion of its child...\n");

    // Detach and remove shared memory
    shmdt((void *) shmPTR);
    printf("Parent has detached shared memory...\n");
    shmctl(shmID, IPC_RMID, NULL);
    printf("Parent has removed shared memory...\n");

    printf("Parent exits...\n");
    exit(0);
}

void ClientProcess(int sharedMem[]) {
    srand(54);  // Seed the random number generator for the client process
    int i = 0;
    int account, balance;
    int sleep_time;

    // Client process performs withdrawals
    while (i < 25) {
        sleep_time = rand() % 6;  // Random sleep time (0 - 5 seconds)
        sleep(sleep_time);

        // Wait until parent has completed a deposit
        while (sharedMem[1] != 1);

        balance = rand() % 51;  // Random withdrawal (0 - 50)
        account = sharedMem[0];

        printf("Poor Student needs $%d\n", balance);

        // Perform withdrawal if account balance is sufficient
        if (balance <= account) {
            account -= balance;
            printf("Poor Student: Withdraws $%d / Balance = $%d\n", balance, account);
        } else {
            printf("Poor Student: Not Enough Cash ($%d)\n", account);
        }

        // Update shared memory with new account balance
        sharedMem[0] = account;

        // Set flag to indicate child transaction is done
        sharedMem[1] = 0;
        i++;
    }

    // Child process exits
    printf("Child Exits\n");
}
