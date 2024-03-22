#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

struct node {
   long int data;
   struct node *next;
};

extern void printList();
extern void insertatbegin(int data);
extern void insertatend(int data);
extern void deleteatbegin();
extern void deleteatend();
extern void deletenode(int key);
extern int searchlist(int key);
