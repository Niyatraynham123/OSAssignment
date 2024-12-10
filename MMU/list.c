// list/list.c
// Implementation for linked list.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "list.h"

list_t *list_alloc() { 
    list_t* list = (list_t*)malloc(sizeof(list_t));
    list->head = NULL;
    return list; 
}

node_t *node_alloc(block_t *blk) {   
    node_t* node = malloc(sizeof(node_t));
    node->next = NULL;
    node->blk = blk;
    return node; 
}

void list_free(list_t *l) {
    node_t *current = l->head;
    node_t *next_node;
    while (current != NULL) {
        next_node = current->next;
        node_free(current);
        current = next_node;
    }
    free(l);
}

void node_free(node_t *node) {
    free(node);
}

void list_print(list_t *l) {
    node_t *current = l->head;
    block_t *b;
    
    if (current == NULL) {
        printf("List is empty\n");
        return;
    }

    while (current != NULL) {
        b = current->blk;
        printf("PID=%d START:%d END:%d\n", b->pid, b->start, b->end);
        current = current->next;
    }
}

int list_length(list_t *l) { 
    node_t *current = l->head;
    int i = 0;
    while (current != NULL) {
        i++;
        current = current->next;
    }
    return i; 
}

void list_add_to_back(list_t *l, block_t *blk) {  
    node_t *newNode = node_alloc(blk);
    newNode->next = NULL;
    
    if (l->head == NULL) {
        l->head = newNode;
    } else {
        node_t *current = l->head;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = newNode;
    }
}

void list_add_to_front(list_t *l, block_t *blk) {  
    node_t *newNode = node_alloc(blk);
    newNode->next = l->head;
    l->head = newNode;
}

void list_add_at_index(list_t *l, block_t *blk, int index) {
    int i = 0;
    node_t *newNode = node_alloc(blk);
    node_t *current = l->head;

    if (index == 0) {
        newNode->next = l->head;
        l->head = newNode;
        return;
    }

    while (i < index - 1 && current != NULL) {
        current = current->next;
        i++;
    }

    if (current == NULL) {
        // Index out of bounds, add to the end
        list_add_to_back(l, blk);
    } else {
        newNode->next = current->next;
        current->next = newNode;
    }
}

void list_add_ascending_by_address(list_t *l, block_t *newblk) {
    node_t* node_block = node_alloc(newblk);
    node_t* curr_node = l->head;
    if (!curr_node) {
        l->head = node_block;
        return;
    }

    node_t* prev = NULL;
    while (curr_node != NULL && node_block->blk->start > curr_node->blk->start) {
        prev = curr_node;
        curr_node = curr_node->next;
    }

    if (prev == NULL) {
        node_block->next = l->head;
        l->head = node_block;
    } else {
        prev->next = node_block;
        node_block->next = curr_node;
    }
}

void list_add_ascending_by_blocksize(list_t *l, block_t *newblk) {
    node_t* node_block = node_alloc(newblk);
    node_t* curr_node = l->head;
    int node_block_size = newblk->end - newblk->start + 1;
    
    if (!curr_node) {
        l->head = node_block;
        return;
    }

    node_t* prev = NULL;
    while (curr_node != NULL && node_block_size > (curr_node->blk->end - curr_node->blk->start + 1)) {
        prev = curr_node;
        curr_node = curr_node->next;
    }

    if (prev == NULL) {
        node_block->next = l->head;
        l->head = node_block;
    } else {
        prev->next = node_block;
        node_block->next = curr_node;
    }
}

void list_add_descending_by_blocksize(list_t *l, block_t *blk) {
    node_t *newNode = node_alloc(blk);
    int newblk_size = blk->end - blk->start;
    
    if (l->head == NULL) {
        l->head = newNode;
    } else {
        node_t *current = l->head;
        node_t *prev = NULL;

        while (current != NULL && (current->blk->end - current->blk->start) >= newblk_size) {
            prev = current;
            current = current->next;
        }

        if (prev == NULL) {
            newNode->next = l->head;
            l->head = newNode;
        } else {
            prev->next = newNode;
            newNode->next = current;
        }
    }
}

void list_coalesce_nodes(list_t *l) {
    node_t* prev = l->head;
    node_t* curr_node = prev ? prev->next : NULL;

    while (curr_node) {
        if (prev->blk->end + 1 == curr_node->blk->start) {
            prev->blk->end = curr_node->blk->end;
            prev->next = curr_node->next;
            node_free(curr_node);
            curr_node = prev->next;
        } else {
            prev = curr_node;
            curr_node = curr_node->next;
        }
    }
}

block_t* list_remove_from_back(list_t *l) {
    if (l->head == NULL) return NULL;
    
    node_t *current = l->head;
    if (current->next == NULL) {
        block_t *value = current->blk;
        node_free(current);
        l->head = NULL;
        return value;
    }

    while (current->next->next != NULL) {
        current = current->next;
    }

    block_t *value = current->next->blk;
    node_free(current->next);
    current->next = NULL;
    return value;
}

block_t* list_get_from_front(list_t *l) {
    if (l->head == NULL) return NULL;
    return l->head->blk;
}

block_t* list_remove_from_front(list_t *l) {
    if (l->head == NULL) return NULL;
    
    node_t *current = l->head;
    block_t *value = current->blk;
    l->head = current->next;
    node_free(current);
    return value;
}

block_t* list_remove_at_index(list_t *l, int index) {
    if (l->head == NULL) return NULL;
    
    if (index == 0) return list_remove_from_front(l);
    
    node_t *current = l->head;
    node_t *prev = NULL;
    int i = 0;
    
    while (current != NULL && i < index) {
        prev = current;
        current = current->next;
        i++;
    }

    if (current == NULL) return NULL; // index out of bounds
    
    prev->next = current->next;
    block_t *value = current->blk;
    node_free(current);
    return value;
}

bool compareBlks(block_t* a, block_t *b) {
    return (a->pid == b->pid && a->start == b->start && a->end == b->end);
}

bool compareSize(int a, block_t *b) {
    return a <= (b->end - b->start + 1);
}

bool comparePid(int a, block_t *b) {
    return a == b->pid;
}

bool list_is_in(list_t *l, block_t* value) {
    node_t *current = l->head;
    while (current != NULL) {
        if (compareBlks(value, current->blk)) {
            return true;
        }
        current = current->next;
    }
    return false;
}

block_t* list_get_elem_at(list_t *l, int index) {
    if (l->head == NULL) return NULL;
    
    node_t *current = l->head;
    int i = 0;
    
    while (current != NULL && i < index) {
        current = current->next;
        i++;
    }
    
    return (current != NULL) ? current->blk : NULL;
}

int list_get_index_of(list_t *l, block_t* value) {
    node_t *current = l->head;
    int index = 0;
    
    while (current != NULL) {
        if (compareBlks(value, current->blk)) {
            return index;
        }
        current = current->next;
        index++;
    }
    
    return -1;
}
