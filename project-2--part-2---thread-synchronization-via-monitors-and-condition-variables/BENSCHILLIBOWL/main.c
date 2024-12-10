#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

#include "BENSCHILLIBOWL.h"

// Configuration parameters for the restaurant simulation
#define RESTAURANT_CAPACITY 100
#define NUM_CUSTOMERS 90
#define NUM_COOKS 10
#define ORDERS_PER_CUSTOMER 3
#define TOTAL_EXPECTED_ORDERS (NUM_CUSTOMERS * ORDERS_PER_CUSTOMER)

// Global variable for the restaurant
BENSCHILLIBOWL *restaurant;

/**
 * Thread function representing a customer.
 * A customer performs the following:
 *  - Allocates memory for an order.
 *  - Selects a random menu item.
 *  - Populates the order with the selected menu item and their customer ID.
 *  - Adds the order to the restaurant.
 */
void* CustomerThread(void* arg) {
    int customer_id = *(int*)arg;

    for (int i = 0; i < ORDERS_PER_CUSTOMER; i++) {
        Order* order = (Order*)malloc(sizeof(Order));
        if (!order) {
            perror("Failed to allocate memory for order");
            pthread_exit(NULL);
        }

        order->menu_item = SelectRandomMenuItem();
        order->customer_id = customer_id;
        order->next = NULL;

        AddOrder(restaurant, order);
    }

    pthread_exit(NULL);
}

/**
 * Thread function representing a cook.
 * A cook performs the following:
 *  - Retrieves an order from the restaurant.
 *  - If the order is valid, fulfills it and frees the associated memory.
 * The cook continues until all expected orders are handled.
 */
void* CookThread(void* arg) {
    int cook_id = *(int*)arg;
    int orders_fulfilled = 0;

    while (1) {
        pthread_mutex_lock(&restaurant->mutex);

        if (restaurant->orders_handled >= restaurant->expected_num_orders) {
            pthread_mutex_unlock(&restaurant->mutex);
            break;
        }

        pthread_mutex_unlock(&restaurant->mutex);

        Order* order = GetOrder(restaurant);
        if (order) {
            printf("Cook #%d fulfilled order #%d from customer #%d\n", 
                   cook_id, order->order_number, order->customer_id);
            free(order);
            orders_fulfilled++;
        }
    }

    printf("Cook #%d fulfilled %d orders\n", cook_id, orders_fulfilled);
    pthread_exit(NULL);
}

/**
 * Main function to execute the restaurant simulation.
 * The program performs the following:
 *  - Opens the restaurant.
 *  - Creates threads for customers and cooks.
 *  - Waits for all threads to complete.
 *  - Closes the restaurant and releases resources.
 */
int main() {
    srand(time(NULL));

    restaurant = OpenRestaurant(RESTAURANT_CAPACITY, TOTAL_EXPECTED_ORDERS);

    pthread_t customer_threads[NUM_CUSTOMERS];
    pthread_t cook_threads[NUM_COOKS];
    int customer_ids[NUM_CUSTOMERS];
    int cook_ids[NUM_COOKS];

    // Create cook threads
    for (int i = 0; i < NUM_COOKS; i++) {
        cook_ids[i] = i + 1;
        if (pthread_create(&cook_threads[i], NULL, CookThread, &cook_ids[i]) != 0) {
            perror("Failed to create cook thread");
            exit(EXIT_FAILURE);
        }
    }

    // Create customer threads
    for (int i = 0; i < NUM_CUSTOMERS; i++) {
        customer_ids[i] = i + 1;
        if (pthread_create(&customer_threads[i], NULL, CustomerThread, &customer_ids[i]) != 0) {
            perror("Failed to create customer thread");
            exit(EXIT_FAILURE);
        }
    }

    // Wait for all customer threads to finish
    for (int i = 0; i < NUM_CUSTOMERS; i++) {
        pthread_join(customer_threads[i], NULL);
    }

    // Wait for all cook threads to finish
    for (int i = 0; i < NUM_COOKS; i++) {
        pthread_join(cook_threads[i], NULL);
    }

    CloseRestaurant(restaurant);

    return 0;
}
