#ifndef QUEUES_H
#define QUEUES_H

#define Q_SIZE (140)
#define RXBUFFER_SIZE (140)

//Queue struct
typedef struct {
  unsigned char Data[Q_SIZE];
  unsigned int Head; // points to oldest data element
  unsigned int Tail; // points to next free space
  unsigned int Size; // quantity of elements in queue
} Q_T;

extern Q_T tx_q, rx_q;

//Queue functions
extern void Q_Init(Q_T * q);
extern int Q_Empty(Q_T * q);
extern int Q_Full(Q_T * q);
extern int Q_Enqueue(Q_T * q, unsigned char d);
extern unsigned char Q_Dequeue(Q_T * q);

//string buffer functions
extern void get_string(void);
extern void send_string(char * buffer);

extern  char rxbuffer[RXBUFFER_SIZE];
  

#endif // QUEUES_H