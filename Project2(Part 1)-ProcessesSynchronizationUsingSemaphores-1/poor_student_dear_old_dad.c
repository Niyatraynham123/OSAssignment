#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <semaphore.h>
#include <fcntl.h>
#include <time.h>

// Function Prototypes
void dear_old_dad(int *bank_account, sem_t *mutex);
void poor_student(int *bank_account, sem_t *mutex);
void lovable_mom(int *bank_account, sem_t *mutex);

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: %s <number_of_parents> <number_of_children>\n", argv[0]);
        exit(1);
    }

    int num_parents = atoi(argv[1]);
    int num_children = atoi(argv[2]);

    // Create shared memory for BankAccount
    int ShmID = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | 0666);
    if (ShmID < 0) {
        perror("shmget error");
        exit(1);
    }
    int *BankAccount = (int *)shmat(ShmID, NULL, 0);
    *BankAccount = 0;  // Initialize the bank account balance

    // Create semaphore for mutual exclusion
    sem_t *mutex = sem_open("/bank_mutex", O_CREAT, 0644, 1);
    if (mutex == SEM_FAILED) {
        perror("sem_open error");
        exit(1);
    }

    // Create parent processes
    for (int i = 0; i < num_parents; i++) {
        if (fork() == 0) {
            if (i == 0) {
                dear_old_dad(BankAccount, mutex);
            } else {
                lovable_mom(BankAccount, mutex);
            }
            exit(0);
        }
    }

    // Create child processes
    for (int i = 0; i < num_children; i++) {
        if (fork() == 0) {
            poor_student(BankAccount, mutex);
            exit(0);
        }
    }

    // Wait for all child processes to finish
    while (wait(NULL) > 0);

    // Cleanup
    shmdt((void *)BankAccount);
    shmctl(ShmID, IPC_RMID, NULL);
    sem_close(mutex);
    sem_unlink("/bank_mutex");

    printf("Simulation completed.\n");
    return 0;
}

void dear_old_dad(int *bank_account, sem_t *mutex) {
    srand(time(NULL) ^ getpid());
    while (1) {
        sleep(rand() % 6);  // Sleep 0-5 seconds
        printf("Dear Old Dad: Attempting to Check Balance\n");

        sem_wait(mutex);
        int local_balance = *bank_account;
        if (rand() % 2 == 0) {  // Even number
            if (local_balance < 100) {
                int deposit = rand() % 100 + 1;  // Deposit 1-100
                local_balance += deposit;
                *bank_account = local_balance;
                printf("Dear Old Dad: Deposits $%d / Balance = $%d\n", deposit, local_balance);
            } else {
                printf("Dear Old Dad: Thinks Student has enough Cash ($%d)\n", local_balance);
            }
        } else {
            printf("Dear Old Dad: Last Checking Balance = $%d\n", local_balance);
        }
        sem_post(mutex);
    }
}

void lovable_mom(int *bank_account, sem_t *mutex) {
    srand(time(NULL) ^ getpid());
    while (1) {
        sleep(rand() % 11);  // Sleep 0-10 seconds
        printf("Lovable Mom: Attempting to Check Balance\n");

        sem_wait(mutex);
        int local_balance = *bank_account;
        if (local_balance <= 100) {
            int deposit = rand() % 125 + 1;  // Deposit 1-125
            local_balance += deposit;
            *bank_account = local_balance;
            printf("Lovable Mom: Deposits $%d / Balance = $%d\n", deposit, local_balance);
        }
        sem_post(mutex);
    }
}

void poor_student(int *bank_account, sem_t *mutex) {
    srand(time(NULL) ^ getpid());
    while (1) {
        sleep(rand() % 6);  // Sleep 0-5 seconds
        printf("Poor Student: Attempting to Check Balance\n");

        sem_wait(mutex);
        int local_balance = *bank_account;
        if (rand() % 2 == 0) {  // Even number
            int need = rand() % 50 + 1;  // Need 1-50
            printf("Poor Student needs $%d\n", need);
            if (need <= local_balance) {
                local_balance -= need;
                *bank_account = local_balance;
                printf("Poor Student: Withdraws $%d / Balance = $%d\n", need, local_balance);
            } else {
                printf("Poor Student: Not Enough Cash ($%d)\n", local_balance);
            }
        } else {
            printf("Poor Student: Last Checking Balance = $%d\n", local_balance);
        }
        sem_post(mutex);
    }
}
