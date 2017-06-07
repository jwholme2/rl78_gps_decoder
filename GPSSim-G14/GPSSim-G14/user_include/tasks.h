#ifndef TASKS_H
#define TASKS_H

extern void Task_GPS_LCD_Update(void);
extern void Task_Clock_LCD_Update(void);
extern void Task_NMEA_Decode(void);
extern void Task_Transmit(void);
extern void Task_SW1(void);

#endif // TASKS_H