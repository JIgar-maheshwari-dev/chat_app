#include<stdio.h>
#include<stdlib.h>

struct Node{
    int *fd;
    struct Node *next;
};

typedef struct Node node;

node *head = NULL;
node *tail = NULL;

int *dequeue();
void enqueue(int *);

void enqueue(int *fd){
    
    node *newnode=(node *)malloc(sizeof(node));
    newnode->fd = fd;
    newnode->next = NULL;
    if(head == NULL ){
        head = newnode;
    }
    else{
        tail->next = newnode;
    }
    tail = newnode;
}

void print_list(){
  if(head == NULL) {
      printf("Queue is empty...\n");
      return;
  }
  node *temp = head;
  while(temp != NULL){
      printf("%d-->",*temp->fd);
      temp = temp->next;
  }
  printf("\n");
}

int *dequeue(){
    if(head == NULL ) {
        return NULL;
    }
    else{
        int *fdp = (head->fd);
        node *temp = head;
        head = head->next;
        if(head == NULL ){ tail = NULL ;}
        free(temp);
        return fdp;
    }
}