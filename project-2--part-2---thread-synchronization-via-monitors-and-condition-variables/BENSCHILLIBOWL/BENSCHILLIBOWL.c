#include "BENSCHILLIBOWL.h"

#include <assert.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <errno.h>

#define MAX_ORDERS 99

// Helper function prototypes
bool IsQueueEmpty(BENSCHILLIBOWL* restaurant);
bool IsQueueFull(BENSCHILLIBOWL* restaurant);
void EnqueueOrder(Order** order_queue, Order* new_order);

// Menu items available at the restaurant
MenuItem RestaurantMenu[] = {
    "BensChilli",
    "BensHalfSmoke",
    "BensHotDog",
    "BensChilliCheeseFries",
    "BensShake",
    "BensHotCakes",
    "BensCake",
    "BensHamburger",
    "BensVeggieBurger",
    "BensOnionRings",
};
int MenuLength = sizeof(RestaurantMenu) / sizeof(MenuItem);

/* Select a random item from the menu */
MenuItem SelectRandomMenuItem() {
    return RestaurantMenu[rand() % MenuLength];
}

/* Initialize and open the restaurant */
BENSCHILLIBOWL* InitializeRestaurant(int max_capacity, int total_expected_orders) {
    printf("Restaurant is now open!\n");

    BENSCHILLIBOWL* restaurant = malloc(sizeof(BENSCHILLIBOWL));
    if (!restaurant) {
        perror("Failed to allocate memory for restaurant");
        exit(1);
    }

    restaurant->orders = NULL;
    restaurant->current_size = 0;
    restaurant->max_size = max_capacity;
    restaurant->expected_num_orders = total_expected_orders;
    restaurant->next_order_number = 1;
    restaurant->orders_handled = 0;

    if (pthread_mutex_init(&restaurant->mutex, NULL) != 0) {
        perror("Mutex initialization failed");
        exit(1);
    }
    if (pthread_cond_init(&restaurant->can_add_orders, NULL) != 0) {
        perror("Condition variable initialization failed");
        exit(1);
    }
    if (pthread_cond_init(&restaurant->can_get_orders, NULL) != 0) {
        perror("Condition variable initialization failed");
        exit(1);
    }

    return restaurant;
}

/* Free all orders in the queue */
void FreeOrderQueue(Order* order) {
    if (order) {
        FreeOrderQueue(order->next);
        free(order);
    }
}

/* Close the restaurant and release all resources */
void ShutdownRestaurant(BENSCHILLIBOWL* restaurant) {
    printf("Restaurant is now closed!\n");

    FreeOrderQueue(restaurant->orders);

    pthread_mutex_destroy(&restaurant->mutex);
    pthread_cond_destroy(&restaurant->can_add_orders);
    pthread_cond_destroy(&restaurant->can_get_orders);

    free(restaurant);
}

/* Add an order to the queue */
int PlaceOrder(BENSCHILLIBOWL* restaurant, Order* new_order) {
    pthread_mutex_lock(&restaurant->mutex);

    while (IsQueueFull(restaurant)) {
        pthread_cond_wait(&restaurant->can_add_orders, &restaurant->mutex);
    }

    new_order->order_number = restaurant->next_order_number++;
    EnqueueOrder(&restaurant->orders, new_order);
    restaurant->current_size++;

    pthread_cond_signal(&restaurant->can_get_orders);
    pthread_mutex_unlock(&restaurant->mutex);

    return new_order->order_number;
}

/* Remove and return an order from the queue */
Order* RetrieveOrder(BENSCHILLIBOWL* restaurant) {
    pthread_mutex_lock(&restaurant->mutex);

    struct timespec timeout;
    struct timeval now;
    gettimeofday(&now, NULL);
    timeout.tv_sec = now.tv_sec + 1;
    timeout.tv_nsec = now.tv_usec * 1000;

    while (IsQueueEmpty(restaurant)) {
        if (pthread_cond_timedwait(&restaurant->can_get_orders, &restaurant->mutex, &timeout) == ETIMEDOUT) {
            pthread_mutex_unlock(&restaurant->mutex);
            return NULL;
        }
    }

    Order* retrieved_order = restaurant->orders;
    restaurant->orders = restaurant->orders->next;
    restaurant->current_size--;
    restaurant->orders_handled++;

    pthread_cond_signal(&restaurant->can_add_orders);
    pthread_mutex_unlock(&restaurant->mutex);

    return retrieved_order;
}

/* Check if the order queue is empty */
bool IsQueueEmpty(BENSCHILLIBOWL* restaurant) {
    return restaurant->current_size == 0;
}

/* Check if the order queue is full */
bool IsQueueFull(BENSCHILLIBOWL* restaurant) {
    return restaurant->current_size == restaurant->max_size;
}

/* Add an order to the end of the queue */
void EnqueueOrder(Order** order_queue, Order* new_order) {
    if (*order_queue == NULL) {
        *order_queue = new_order;
        new_order->next = NULL;
        return;
    }

    Order* temp = *order_queue;
    while (temp->next != NULL) {
        temp = temp->next;
    }
    temp->next = new_order;
    new_order->next = NULL;
}
