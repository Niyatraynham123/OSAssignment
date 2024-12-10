#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "list.h"
#include "util.h"

// Convert all characters in the string to uppercase
void to_uppercase(char *str) {
    for (int i = 0; i < strlen(str); i++) {
        str[i] = toupper(str[i]);
    }
}

// Process input arguments and configure memory management policy
void process_input(char *args[], int input[][2], int *num_operations, int *partition_size, int *policy) {
    FILE *input_file = fopen(args[1], "r");
    if (!input_file) {
        fprintf(stderr, "Error: Invalid filepath\n");
        exit(EXIT_FAILURE);
    }

    parse_file(input_file, input, num_operations, partition_size);
    fclose(input_file);

    to_uppercase(args[2]);

    if (strcmp(args[2], "-F") == 0 || strcmp(args[2], "-FIFO") == 0)
        *policy = 1;
    else if (strcmp(args[2], "-B") == 0 || strcmp(args[2], "-BESTFIT") == 0)
        *policy = 2;
    else if (strcmp(args[2], "-W") == 0 || strcmp(args[2], "-WORSTFIT") == 0)
        *policy = 3;
    else {
        printf("usage: ./mmu <input file> -{F | B | W}\n(F=FIFO | B=BESTFIT | W=WORSTFIT)\n");
        exit(EXIT_FAILURE);
    }
}

// Allocate memory for a process based on the selected policy
void allocate_memory(list_t *free_list, list_t *alloc_list, int pid, int block_size, int policy) {
    block_t *block;
    block_t *fragment = malloc(sizeof(block_t));

    if (list_is_in_by_size(free_list, block_size)) {
        int index = list_get_index_of_by_Size(free_list, block_size);
        block = list_remove_at_index(free_list, index);
    } else {
        printf("Error: Memory Allocation %d blocks\n", block_size);
        return;
    }

    block->pid = pid;
    fragment->end = block->end;
    block->end = block->start + block_size - 1;
    list_add_ascending_by_address(alloc_list, block);

    fragment->pid = 0;
    fragment->start = block->end + 1;

    // Add the fragment to the free list based on the policy
    if (policy == 1) {
        list_add_to_back(free_list, fragment);
    } else if (policy == 2) {
        list_add_ascending_by_blocksize(free_list, fragment);
    } else if (policy == 3) {
        list_add_descending_by_blocksize(free_list, fragment);
    }
}

// Deallocate memory for a process and return it to the free list
void deallocate_memory(list_t *alloc_list, list_t *free_list, int pid, int policy) {
    block_t *block;

    int index = list_get_index_of_by_Pid(alloc_list, pid);
    if (index == -1) {
        printf("Error: Can't locate Memory Used by PID: <%d>\n", pid);
        return;
    }
    block = list_remove_at_index(alloc_list, index);
    block->pid = 0;

    // Add the block back to the free list based on the policy
    if (policy == 1) {
        list_add_to_back(free_list, block);
    } else if (policy == 2) {
        list_add_ascending_by_blocksize(free_list, block);
    } else if (policy == 3) {
        list_add_descending_by_blocksize(free_list, block);
    }
}

// Coalesce adjacent free memory blocks
list_t* coalesce_memory(list_t *list) {
    list_t *temp_list = list_alloc();
    block_t *block;

    while ((block = list_remove_from_front(list)) != NULL) {
        list_add_ascending_by_address(temp_list, block);
    }

    list_coalesce_nodes(temp_list);
    return temp_list;
}

// Print the memory blocks in a list
void print_memory_list(list_t *list, const char *message) {
    node_t *current = list->head;
    block_t *block;
    int index = 0;

    printf("%s:\n", message);

    while (current != NULL) {
        block = current->blk;
        printf("Block %d:\t START: %d\t END: %d", index, block->start, block->end);

        if (block->pid != 0)
            printf("\t PID: %d\n", block->pid);
        else  
            printf("\n");

        current = current->next;
        index++;
    }
}

int main(int argc, char *argv[]) {
    int partition_size, input_data[200][2], num_operations = 0, memory_mgmt_policy;
    list_t *free_list = list_alloc();
    list_t *alloc_list = list_alloc();

    if (argc != 3) {
        printf("usage: ./mmu <input file> -{F | B | W}\n(F=FIFO | B=BESTFIT | W=WORSTFIT)\n");
        exit(EXIT_FAILURE);
    }

    process_input(argv, input_data, &num_operations, &partition_size, &memory_mgmt_policy);

    // Initialize the partition of memory
    block_t *partition = malloc(sizeof(block_t));
    partition->start = 0;
    partition->end = partition_size + partition->start - 1;

    list_add_to_front(free_list, partition);

    // Simulate memory operations
    for (int i = 0; i < num_operations; i++) {
        printf("************************\n");
        if (input_data[i][0] != -99999 && input_data[i][0] > 0) {
            printf("ALLOCATE: %d FROM PID: %d\n", input_data[i][1], input_data[i][0]);
            allocate_memory(free_list, alloc_list, input_data[i][0], input_data[i][1], memory_mgmt_policy);
        } else if (input_data[i][0] != -99999 && input_data[i][0] < 0) {
            printf("DEALLOCATE MEM: PID %d\n", abs(input_data[i][0]));
            deallocate_memory(alloc_list, free_list, abs(input_data[i][0]), memory_mgmt_policy);
        } else {
            printf("COALESCE/COMPACT\n");
            free_list = coalesce_memory(free_list);
        }

        printf("************************\n");
        print_memory_list(free_list, "Free Memory");
        print_memory_list(alloc_list, "\nAllocated Memory");
        printf("\n\n");
    }

    list_free(free_list);
    list_free(alloc_list);

    return 0;
}
