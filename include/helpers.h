#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>
#include "icssh.h"
#include <time.h>
#include <signal.h>




typedef struct node_t{
    bgentry_t* bgentry;
    struct node_t* next;
} node_t;

typedef struct list {
    node_t* head;
    int length;
} list_t;


list_t* CreateList();
void InsertNodeAtHead(list_t* list, node_t* newNode);
void InsertNodeAtTail(list_t* list, node_t* newNode);
void PrintLinkedList(list_t* list, FILE* fp);
node_t* RemoveByPid(node_t** head, pid_t pid);
node_t* RemoveFromHead(node_t** head);


void print_date_and_time(int signum);