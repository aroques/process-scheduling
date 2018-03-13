#include <stdbool.h>

#include "shared_memory.h"

#define MAX PROC_CTRL_TBL_SZE

struct Queue {
    int arr[MAX];
    int front;      // Init to 0
    int rear;       // Init to -1
    int itemCount;  // Init to 0
};

int peek(struct Queue q);
bool isEmpty(struct Queue q);
bool isFull(struct Queue q);
void insert(struct Queue* q, int val);
int dequeue(struct Queue* q);