#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

#define MAX 20  // Define the matrix size

int matA[MAX][MAX];
int matB[MAX][MAX];
int matSumResult[MAX][MAX];
int matDiffResult[MAX][MAX];
int matProductResult[MAX][MAX];

pthread_mutex_t mutex; // Mutex for thread synchronization

// Function to fill a matrix with random values
void fillMatrix(int matrix[MAX][MAX]) {
    for (int i = 0; i < MAX; i++) {
        for (int j = 0; j < MAX; j++) {
            matrix[i][j] = rand() % 10 + 1;
        }
    }
}

// Function to print a matrix
void printMatrix(int matrix[MAX][MAX]) {
    for (int i = 0; i < MAX; i++) {
        for (int j = 0; j < MAX; j++) {
            printf("%5d", matrix[i][j]);
        }
        printf("\n");
    }
    printf("\n");
}

// Structure to pass data to each thread (for each matrix cell)
typedef struct {
    int row;
    int col;
} ThreadData;

// Compute the sum of corresponding elements of matA and matB
void* computeSum(void* args) {
    ThreadData* data = (ThreadData*)args;
    int row = data->row;
    int col = data->col;
    
    matSumResult[row][col] = matA[row][col] + matB[row][col];
    
    free(data);  // Free memory allocated for thread data
    return NULL;
}

// Compute the difference of corresponding elements of matA and matB
void* computeDiff(void* args) {
    ThreadData* data = (ThreadData*)args;
    int row = data->row;
    int col = data->col;
    
    matDiffResult[row][col] = matA[row][col] - matB[row][col];
    
    free(data);  // Free memory allocated for thread data
    return NULL;
}

// Compute the product of corresponding row of matA and column of matB (dot product)
void* computeProduct(void* args) {
    ThreadData* data = (ThreadData*)args;
    int row = data->row;
    int col = data->col;
    
    matProductResult[row][col] = 0;
    for (int i = 0; i < MAX; i++) {
        matProductResult[row][col] += matA[row][i] * matB[i][col];
    }

    free(data);  // Free memory allocated for thread data
    return NULL;
}

int main() {
    srand(time(0));  // Initialize random number generator

    // 1. Fill the matrices (matA and matB) with random values
    fillMatrix(matA);
    fillMatrix(matB);

    // 2. Print the initial matrices
    printf("Matrix A:\n");
    printMatrix(matA);
    printf("Matrix B:\n");
    printMatrix(matB);

    // 3. Create pthread_t objects for the threads
    pthread_t threads[10];
    int threadCount = 0;

    // 4. Create a thread for each cell in each matrix operation
    for (int i = 0; i < MAX; i++) {
        for (int j = 0; j < MAX; j++) {
            // Create thread data for each matrix element (row, col)
            ThreadData* data = (ThreadData*)malloc(sizeof(ThreadData));
            data->row = i;
            data->col = j;

            // For sum
            pthread_create(&threads[threadCount], NULL, computeSum, data);
            pthread_join(threads[threadCount], NULL);
            threadCount++;

            // For difference
            pthread_create(&threads[threadCount], NULL, computeDiff, data);
            pthread_join(threads[threadCount], NULL);
            threadCount++;

            // For product
            pthread_create(&threads[threadCount], NULL, computeProduct, data);
            pthread_join(threads[threadCount], NULL);
            threadCount++;
        }
    }

    // 5. Wait for all threads to finish (handled by pthread_join)
    
    // 6. Print the results
    printf("Results:\n");
    printf("Sum:\n");
    printMatrix(matSumResult);
    printf("Difference:\n");
    printMatrix(matDiffResult);
    printf("Product:\n");
    printMatrix(matProductResult);

    return 0;
}
