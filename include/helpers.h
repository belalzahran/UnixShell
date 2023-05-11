#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>
#include "icssh.h"



typedef struct node_t {
    bgentry_t* bgentry;
    struct node_t* next;
} node_t;

typedef struct list {
    node_t* head;
    int length;
} list_t;


list_t* CreateList()
{
    list_t* list = malloc(sizeof(list_t));
    list->length = 0;
    list->head = NULL;
    return list;

}

void InsertAtHead(list_t* list, node_t* newNode) {
    if(list == NULL || newNode == NULL)
        return;
    if (list->length == 0) 
        list->head = NULL;

    node_t** head = &(list->head);
    node_t* new_node;
    new_node = malloc(sizeof(node_t));

    new_node->bgentry = newNode->bgentry;

    new_node->next = *head;

    // moves list head to the new node
    *head = new_node;
    list->length++;
}

void PrintLinkedList(list_t* list, FILE* fp) {
    if(list == NULL)
        return;

    node_t* head = list->head;
    while (head != NULL) {
        print_bgentry(head->bgentry);
        head = head->next;
    }
}

node_t* RemoveByPid(node_t** head, pid_t pid) {
    node_t* removed_node = NULL;

    // Special case: removing from the front of the list
    while (*head != NULL && (*head)->bgentry->pid == pid) {
        removed_node = *head;
        *head = (*head)->next;
        return removed_node;
    }

    // General case: removing from somewhere else in the list
    node_t* current = *head;
    while (current != NULL && current->next != NULL) {
        if (current->next->bgentry->pid == pid) {
            removed_node = current->next;
            current->next = current->next->next;
            return removed_node;
        } else {
            current = current->next;
        }
    }

    return NULL;
}