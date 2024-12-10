#include<stdio.h>
#include<limits.h>
#include<stdlib.h>
#include "process.h"
#include "util.h"

// Function to find the waiting time for all processes using Round Robin scheduling
void calculateWaitingTimeRR(ProcessType plist[], int n, int quantum) {
    int *remaining_burst_times = malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) {
        remaining_burst_times[i] = plist[i].bt;
        plist[i].wt = 0;
    }

    int current_time = 0;
    int completed_processes = 0;

    // Round Robin scheduling
    while (completed_processes < n) {
        int progress_made = 0;  // Flag to check if any process was completed in this round
        for (int i = 0; i < n; i++) {
            if (remaining_burst_times[i] > 0) {
                progress_made = 1;
                if (remaining_burst_times[i] > quantum) {
                    current_time += quantum;
                    remaining_burst_times[i] -= quantum;
                } else {
                    current_time += remaining_burst_times[i];
                    plist[i].wt = current_time - plist[i].bt - plist[i].art;
                    remaining_burst_times[i] = 0;
                    completed_processes++;
                }
            }
        }
        // If no progress is made in a round, break to avoid infinite loop
        if (!progress_made) break;
    }

    // Calculate turnaround times
    for (int i = 0; i < n; i++) {
        plist[i].tat = plist[i].wt + plist[i].bt;
    }

    free(remaining_burst_times);
}

// Function to find the waiting time for all processes using First Come First Serve scheduling
void calculateWaitingTimeFCFS(ProcessType plist[], int n) {
    plist[0].wt = plist[0].art; // Waiting time for first process is its arrival time
    for (int i = 1; i < n; i++) {
        plist[i].wt = plist[i - 1].bt + plist[i - 1].wt;
    }
}

// Function to find the waiting time for all processes using Shortest Job First scheduling
void calculateWaitingTimeSJF(ProcessType plist[], int n) {
    int completed_processes = 0;
    int time_lap = 0;
    int *completion_times = malloc(n * sizeof(int));
    int *remaining_times = malloc(n * sizeof(int));

    for (int i = 0; i < n; i++) {
        remaining_times[i] = plist[i].bt;
        completion_times[i] = -1;
    }

    while (completed_processes < n) {
        int min_remaining_time = INT_MAX;
        int selected_process = -1;

        // Find the process with the minimum burst time that has arrived
        for (int i = 0; i < n; i++) {
            if (remaining_times[i] < min_remaining_time && completion_times[i] == -1 && plist[i].art <= time_lap) {
                selected_process = i;
                min_remaining_time = remaining_times[i];
            }
        }

        // Update remaining burst times and completion times
        if (selected_process != -1) {
            remaining_times[selected_process] -= 1;
            if (remaining_times[selected_process] == 0) {
                completed_processes++;
                completion_times[selected_process] = time_lap + 1;
            }
        }
        time_lap++;
    }

    for (int i = 0; i < n; i++) {
        plist[i].wt = completion_times[i] - plist[i].art - plist[i].bt;
    }

    free(completion_times);
    free(remaining_times);
}

// Function to calculate turnaround time for all processes
void calculateTurnAroundTime(ProcessType plist[], int n) {
    for (int i = 0; i < n; i++) {
        plist[i].tat = plist[i].bt + plist[i].wt;
    }
}

// Function to compare processes by priority for sorting
int compareByPriority(const void *a, const void *b) {
    return -1 * (((ProcessType*)a)->pri - ((ProcessType*)b)->pri);
}

// Function to calculate average time for FCFS scheduling
void calculateAverageTimeFCFS(ProcessType plist[], int n) {
    calculateWaitingTimeFCFS(plist, n);
    calculateTurnAroundTime(plist, n);
    printf("\n*********\nFCFS\n");
}

// Function to calculate average time for SJF scheduling
void calculateAverageTimeSJF(ProcessType plist[], int n) {
    calculateWaitingTimeSJF(plist, n);
    calculateTurnAroundTime(plist, n);
    printf("\n*********\nSJF\n");
}

// Function to calculate average time for Round Robin scheduling
void calculateAverageTimeRR(ProcessType plist[], int n, int quantum) {
    calculateWaitingTimeRR(plist, n, quantum);
    calculateTurnAroundTime(plist, n);
    printf("\n*********\nRR Quantum = %d\n", quantum);
}

// Function to calculate average time for Priority scheduling
void calculateAverageTimePriority(ProcessType plist[], int n) {
    qsort(plist, n, sizeof(plist[0]), compareByPriority);
    calculateWaitingTimeFCFS(plist, n); // Use FCFS after sorting by priority
    calculateTurnAroundTime(plist, n);
    printf("\n*********\nPriority\n");
}

// Function to print the metrics (waiting time, turnaround time)
void printMetrics(ProcessType plist[], int n) {
    int total_wt = 0, total_tat = 0;
    float avg_wt, avg_tat;

    printf("\tProcesses\tBurst time\tWaiting time\tTurnaround time\n");

    for (int i = 0; i < n; i++) {
        total_wt += plist[i].wt;
        total_tat += plist[i].tat;
        printf("\t%d\t\t%d\t\t%d\t\t%d\n", plist[i].pid, plist[i].bt, plist[i].wt, plist[i].tat);
    }

    avg_wt = (float)total_wt / n;
    avg_tat = (float)total_tat / n;

    printf("\nAverage waiting time = %.2f", avg_wt);
    printf("\nAverage turnaround time = %.2f\n", avg_tat);
}

// Function to initialize process list from a file
ProcessType* initProcessList(char *filename, int *n) {
    FILE *input_file = fopen(filename, "r");
    if (!input_file) {
        fprintf(stderr, "Error: Invalid filepath\n");
        exit(0);
    }

    ProcessType *plist = parse_file(input_file, n);
    fclose(input_file);

    return plist;
}

// Driver code
int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: ./schedsim <input-file-path>\n");
        return 1;
    }

    int n;
    int quantum = 2;
    ProcessType *proc_list;

    // FCFS
    n = 0;
    proc_list = initProcessList(argv[1], &n);
    calculateAverageTimeFCFS(proc_list, n);
    printMetrics(proc_list, n);

    // SJF
    n = 0;
    proc_list = initProcessList(argv[1], &n);
    calculateAverageTimeSJF(proc_list, n);
    printMetrics(proc_list, n);

    // Priority
    n = 0;
    proc_list = initProcessList(argv[1], &n);
    calculateAverageTimePriority(proc_list, n);
    printMetrics(proc_list, n);

    // Round Robin
    n = 0;
    proc_list = initProcessList(argv[1], &n);
    calculateAverageTimeRR(proc_list, n, quantum);
    printMetrics(proc_list, n);

    return 0;
}

