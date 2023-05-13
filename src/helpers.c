#include "helpers.h"

list_t* CreateList()
{
    list_t* list = malloc(sizeof(list_t));
    list->length = 0;
    list->head = NULL;
    return list;

}

void InsertNodeAtTail(list_t* list, node_t* newNode) {
    if (list == NULL || newNode == NULL)
        return;
    if (list->length == 0) {
        InsertNodeAtHead(list, newNode);
        return;
    }

    node_t* head = list->head;
    node_t* current = head;
    while (current->next != NULL) {
        current = current->next;
    }

    current->next = malloc(sizeof(node_t));
    current->next->bgentry = newNode->bgentry;
    current->next->next = NULL;
    list->length++;
}

void InsertNodeAtHead(list_t* list, node_t* newNode) {
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
    while (*head != NULL && (*head)->bgentry->pid == pid) 
    {
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


node_t* RemoveFromHead(node_t** head) {
    node_t* removed_node = NULL;

    // Check if the list is empty
    if (*head == NULL) {
        return NULL;
    }

    // Remove the node from the head of the list
    removed_node = *head;
    *head = (*head)->next;

    return removed_node;
}

void print_date_and_time(int signum) {
    time_t rawtime;
    struct tm * timeinfo;
    char buffer[80];

    time ( &rawtime );
    timeinfo = localtime ( &rawtime );

    // Format the timeinfo structure into a string.
    strftime(buffer, 80, "%a %b %d %H:%M:%S %Y", timeinfo);

    printf("Current local time and date: %s\n", buffer);
}
