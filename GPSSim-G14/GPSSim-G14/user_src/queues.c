#include "r_cg_macrodriver.h"
#include "r_cg_userdefine.h"

#include "rtc_sched.h"
#include "lcd.h"
#include "queues.h"
#include "stdio.h"
#include "string.h"

//This is used to store the string from the rx_buffer
char rxbuffer[RXBUFFER_SIZE];
//char txbuffer[128];


//Queue functions, courtesy of Dr. Dean
void Q_Init(Q_T * q) {
  unsigned int i;
  for (i=0; i<Q_SIZE; i++)
    q->Data[i] = 0; // to simplify our lives when debugging
  q->Head = 0;
  q->Tail = 0;
  q->Size = 0;
}


int Q_Empty(Q_T * q) {
  return q->Size == 0;
}
int Q_Full(Q_T * q) {
  return q->Size == Q_SIZE;
}

int Q_Enqueue(Q_T * q, unsigned char d) {
  // What if queue is full?
  if (!Q_Full(q)) {
    q->Data[q->Tail++] = d;
    q->Tail %= Q_SIZE;
    q->Size++;
    return 1; // success
  } else
    return 0; // failure
}

unsigned char Q_Dequeue(Q_T * q) {
  // Must check to see if queue is empty before dequeueing
  unsigned char t=0;
  if (!Q_Empty(q)) {
    t = q->Data[q->Head];
    q->Data[q->Head++] = 0; // to simplify debugging
    q->Head %= Q_SIZE;
    q->Size--;
  }
  return t;
}

//Function that retrieves a string from the rx buffer
void get_string(void) {  
  //As long as there is data...
  while (!Q_Empty(&rx_q)) {
     unsigned char ret =  Q_Dequeue(&rx_q);
     //Ugly
     strncat(rxbuffer,  (const char *) &ret,1);
     
     //Ensure that we always have a null-terminator
     rxbuffer[RXBUFFER_SIZE-1]='\0';
  }
}

//Function that places a char string into tx buffer
void send_string(char * buffer) {
  //Continue as long as the queue is not full
  //Or we have a message terminator \n
  for (int i=0; buffer[i]!='\0'; i++) {
    Q_Enqueue(&tx_q, buffer[i]);
  }
}