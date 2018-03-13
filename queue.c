/*
 *  Queue implmentation - adpated from https://www.tutorialspoint.com/data_structures_algorithms/queue_program_in_c.htm
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include "queue.h"

int peek(struct Queue q) {
   return q.arr[q.front];
}

bool isEmpty(struct Queue q) {
   return q.itemCount == 0;
}

bool isFull(struct Queue q) {
   return q.itemCount == MAX;
}

void insert(struct Queue* q, int val) {

   if(!isFull(*q)) {
	
      if(q->rear == MAX-1) {
         q->rear = -1;            
      }       

      q->arr[++q->rear] = val;
      q->itemCount++;
   }
}

int dequeue(struct Queue* q) {
   int val = q->arr[q->front++];
	
   if(q->front == MAX) {
      q->front = 0;
   }
	
   q->itemCount--;
   return val;  
}
