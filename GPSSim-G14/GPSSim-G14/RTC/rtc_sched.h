#ifndef RTC_SCHED_H
#define RTC_SCHED_H

#ifndef NULL
#define NULL (0)
#endif

#define MAX_TASKS 			10 // Set maximum number of tasks to be used in system
                           // Will affect performance.

#define RTC_TICK_FREQ  (1000) // Tick frequency in Hz, assuming using interval
													//	timer with 15 kHz clock and compare value of 14

#define RTC_FREQ_TO_TICKS(f)  (RTC_TICK_FREQ/f) // Macro used to set task period

// power saving options
#define RTC_STOP_WHEN_IDLE (0)
#define RTC_HALT_WHEN_IDLE (0)

// debugging options
#define RTC_MONITOR_ACTIVITY (1)
#define RTC_ACTIVE_OUTPUT 			LED_15_O
#define RTC_ACTIVE_OUTPUT_MODE 	LED_15_O_M

#define RTC_MONITOR_STANDBY (0)
#define RTC_STANDBY_OUTPUT 			LED_5_R
#define RTC_STANDBY_OUTPUT_MODE 	LED5_R_M


/**** Run to completion scheduler API ************************************************/
extern void tick_timer_intr(void);
extern void Init_RTC_Scheduler(void);
extern void Init_Task_Timers(void);
extern int  Add_Task(void (*task)(void), int period, int priority);
extern void Remove_Task(void (*task)(void));
extern void Run_RTC_Scheduler(void);
extern void Run_TaskN(int task_number);
extern void Reschedule_TaskN(int task_number, int new_period);
extern void Enable_TaskN(int task_number);
extern void Disable_TaskN(int task_number);

extern void Reschedule_Task(void (*task)(void), int new_period);
extern void Enable_Task(void (*task)(void));
extern void Disable_Task(void (*task)(void));


//Statistics struct
typedef struct {
  unsigned long min;
  unsigned long max;
  unsigned long total;
  unsigned int num_samples;
} statistics_t;

//Statistics struct
typedef struct {
  signed long start;
  signed long end;
} timer_t;

typedef struct {
  int period;
  int delay;
  int ready;
  int enabled;
  void (* task)(void);
  statistics_t exec_time_us;
  statistics_t latency_us;
  timer_t lat_timer;
} task_t;

extern task_t GBL_task_list[MAX_TASKS];

#endif // #ifndef RTC_SCHED_H
