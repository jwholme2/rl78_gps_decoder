#include "r_cg_macrodriver.h"
#include "r_cg_userdefine.h"

#include "rtc_sched.h"
#include "r_cg_serial.h"
#include "lcd.h"
#include "string.h"
#include "ctype.h"

//JWH
#include "queues.h"
#include "gpssim.h"
#include "limits.h"

//Our parserState definitions
#define START 0
#define TALKER 1
#define UTC 2
#define STATUS 3
#define LAT_DEG 4
#define LAT_DIR 5
#define LONG_DEG 6
#define LONG_DIR 7
#define SPEED 8
#define TRACK 9
#define DATE 10
#define MAG_VAR 11
#define MAG_DIR 12
#define POS_SYS 13
#define CHECKSUM1 14
#define CHECKSUM2 15
#define LAT_MIN 16
#define LONGI_MIN 17

//UTC max size is 10, but min num of chars we need is 6 hhmmss
#define UTC_SIZE 10
#define UTC_MIN 6

//LAT degrees is always 2 digits wide, but 1 extra for null-terminator
#define LAT_DEG_SIZE 3
#define LAT_DEG_MIN 2


#define LAT_MIN_SIZE 9 
#define LAT_MIN_MIN 2 

//DIR will always be 1 char + null
#define LAT_DIR_SIZE 2
#define LAT_DIR_MIN 1


#define LONG_DEG_SIZE 4
#define LONG_DEG_MIN 3


#define LONG_MIN_SIZE 9
#define LONG_MIN_MIN 2

#define LONG_DIR_SIZE 2
#define LONG_DIR_MIN 1


#define SPEED_SIZE 10
#define SPEED_MIN 1

#define TRACK_SIZE 10
#define TRACK_MIN 1

#define DATE_SIZE 2

#define CHECKSUM_SIZE 3

extern volatile uint8_t G_UART_SendingData, G_UART_ReceivingData;


//Holds the UTC time, up to 9 digits (hhmmsssss) + terminator
char utc[UTC_SIZE];
unsigned int utc_i;
//Latitude, up to 8 digits (including decimal)
char lat_deg[LAT_DEG_SIZE];
unsigned int lat_deg_i;

//Latitude minutes, up to 8 digits (including decimal)
char lat_min[LAT_MIN_SIZE];
float lat_min_f;

//Latitude direction, up to 2 char (including \n)
char lat_dir[LAT_DIR_SIZE];

//Longitude, up to 9 digits
char long_deg[LONG_DEG_SIZE];
unsigned int long_deg_i;

//Latitude minutes, up to 8 digits (including decimal)
char long_min[LONG_MIN_SIZE];
float long_min_f;

//Longitude direction, up to 2 char (including \n)
char long_dir[LONG_DIR_SIZE];

//Speed, up to 5 digits
char speed[SPEED_SIZE];
float speed_f;

//Track, up to 5 digits
char track[TRACK_SIZE];
float track_f;

//Checksum, up to 2 digits ddmmyy
char checksum2[CHECKSUM_SIZE];

// Task_Clock_LCD_Update runs at 10 Hz, updates elapsed time on LCD
void Task_Clock_LCD_Update(void) {
  static unsigned char h=0, m=0, s=0, ds=0;

  ds++;
  if (ds>9) {
    s++;
    ds=0;
  }
  
  if (s>59) {
    m++;
    s = 0;
  }
  if (m>59) {
    h++;
    m=0;
  }
  
  //Display my unityID
  LCDPrintf(0,0, "%s", "jwholme2");
                      
  //hh:mm:ss:d
  LCDPrintf(2,0, "%02u:%02u:%02u.%01u", h, m, s, ds);
}

//Task is called by the decode task once there is data to display!
//We use the char arrays, but we can easily use the int or floats defined for
//each variable - we just need to change the printf around
void Task_GPS_LCD_Update(void) {  
  
  //Clear lines
  LCDClearLine(3);
  LCDClearLine(4);
  LCDClearLine(5);
  LCDClearLine(6);
  LCDClearLine(7);
  
  LCDPrintf(3,0, "%s %s %s", lat_deg,lat_min, lat_dir);
  LCDPrintf(4,0, "%s %s %s", long_deg,long_min, long_dir);  
  LCDPrintf(5,0, "%s kt", speed);
  LCDPrintf(6,0, "%s T", track);
  LCDPrintf(7,0, "%c%c:%c%c:%c%c", utc[0], utc[1], utc[2], utc[3], utc[4],utc[5],utc[6]);
  
  //Once printed...clear out all the arrays for next time
  memset(&lat_deg[0], 0, sizeof(lat_deg));  
  memset(&lat_min[0], 0, sizeof(lat_min));
  memset(&lat_dir[0], 0, sizeof(lat_dir));
  memset(&long_deg[0], 0, sizeof(long_deg));      
  memset(&long_min[0], 0, sizeof(long_min));
  memset(&long_dir[0], 0, sizeof(long_dir));
  memset(&speed[0], 0, sizeof(speed));
  memset(&track[0], 0, sizeof(track));
  memset(&utc[0], 0, sizeof(utc));
}

//This task was used to test with as 
//I had issues with my G13-G14 board hookup initially
//Thanks to Dr Dean for helping solve!
void Task_Transmit(void) {        

  static unsigned char bit = 0;
  
 sim_motion();
  //We are sending data
//  if (bit) {
//    sim_motion();
//    bit=0;
//} else {
//    sim_motionGLL();
//    bit=1;
//}

  G_UART_SendingData = 1;	     
  R_UART1_Send();  

  //Wait until data is sent
  while (G_UART_SendingData)
  ;

}

//This task handles printing the task latency/exec info
//when SW1 is pressed.
//This is pretty ugly code ... but since we have a finite number
//of tasks...we can get away with this. Basically we 
//put a delay between each screen. 
void Task_SW1(void) {        

  volatile int32_t n;
  //Clear the screen
  LCDClear();
  
  //As long as the switch is pressed (SW_1 is n.c.)
  while (!SW_1) {
    
    //DECODE task
    LCDPrintf(0,0,"%s","Task:Decode");
         
    //Keep the screen up for a bit...	
    for (n=0; n<1000000; n++)
      ;
    
    //Print latency info
    LCDClear();
    LCDPrintf(0,0,"%s", "LATENCY");
    LCDPrintf(1,0,"%s,%u", "MEAN", (GBL_task_list[1].latency_us.total/GBL_task_list[1].latency_us.num_samples));
    LCDPrintf(2,0,"%s,%u", "MIN", GBL_task_list[1].latency_us.min);
    LCDPrintf(3,0,"%s,%u", "MAX", GBL_task_list[1].latency_us.max);
      
    //Keep the screen up for a bit...
    for (n=0; n<1000000; n++)
      ;
    
    //Print execution info
    LCDClear();
    LCDPrintf(0,0,"%s", "EXECUTION");
    LCDPrintf(1,0,"%s,%u", "MEAN", (GBL_task_list[1].exec_time_us.total/GBL_task_list[1].exec_time_us.num_samples));
    LCDPrintf(2,0,"%s,%u", "MIN", GBL_task_list[1].exec_time_us.min);
    LCDPrintf(3,0,"%s,%u", "MAX", GBL_task_list[1].exec_time_us.max);
    
    //Keep the screen up for a bit...
    for (n=0; n<1000000; n++)
      ;
    
        
    //GPS_LCD_Update task 
    LCDClear();
    LCDPrintf(0,0,"%s","Task");
    LCDPrintf(1,0,"%s","GPS_Update");

    //Keep the screen up for a bit...
    for (n=0; n<1000000; n++)
      ;
    
    //Print latency info
    LCDClear();
    LCDPrintf(0,0,"%s", "LATENCY");
    LCDPrintf(1,0,"%s,%u", "MEAN", (GBL_task_list[2].latency_us.total/GBL_task_list[2].latency_us.num_samples));
    LCDPrintf(2,0,"%s,%u", "MIN", GBL_task_list[2].latency_us.min);
    LCDPrintf(3,0,"%s,%u", "MAX", GBL_task_list[2].latency_us.max);
      
    //Keep the screen up for a bit...
    for (n=0; n<1000000; n++)
      ;
        
    //Print execution info
    LCDClear();
    LCDPrintf(0,0,"%s", "EXECUTION");
    LCDPrintf(1,0,"%s,%u", "MEAN", (GBL_task_list[2].exec_time_us.total/GBL_task_list[2].exec_time_us.num_samples));
    LCDPrintf(2,0,"%s,%u", "MIN", GBL_task_list[2].exec_time_us.min);
    LCDPrintf(3,0,"%s,%u", "MAX", GBL_task_list[2].exec_time_us.max);
    
    //Keep the screen up for a bit...
    for (n=0; n<1000000; n++)
      ;
         
    //Task_Clock_LCD_Update task 
    LCDClear();
    LCDPrintf(0,0,"%s","Task:");
    LCDPrintf(1,0,"%s","Clk_Upd");
                
    //Keep the screen up for a bit...
    for (n=0; n<1000000; n++)
      ;
        
    //Print latency info
    LCDClear();
    LCDPrintf(0,0,"%s", "LATENCY");
    LCDPrintf(1,0,"%s,%u", "MEAN", (GBL_task_list[3].latency_us.total/GBL_task_list[2].latency_us.num_samples));
    LCDPrintf(2,0,"%s,%u", "MIN", GBL_task_list[3].latency_us.min);
    LCDPrintf(3,0,"%s,%u", "MAX", GBL_task_list[3].latency_us.max);
      
    //Keep the screen up for a bit...
    for (n=0; n<1000000; n++)
      ;
        
    //Print execution info
    LCDClear();
    LCDPrintf(0,0,"%s", "EXECUTION");
    LCDPrintf(1,0,"%s,%u", "MEAN", (GBL_task_list[3].exec_time_us.total/GBL_task_list[2].exec_time_us.num_samples));
    LCDPrintf(2,0,"%s,%u", "MIN", GBL_task_list[3].exec_time_us.min);
    LCDPrintf(3,0,"%s,%u", "MAX", GBL_task_list[3].exec_time_us.max);
    
    //Keep the screen up for a bit...
    for (n=0; n<1000000; n++)
      ;
       
  }

  //Once we are done...clear everything
  LCDClear();
}

//This task is used to decode the NMEA message
void Task_NMEA_Decode(void) {
  static unsigned int n = 0;
  static int flag = 0;
  static unsigned int parserState = 0;
  static uint8_t cs = 0; 
  
  //Copy from rx_q to rxbuffer char array
  get_string();
    
  for (int ii=0; rxbuffer[ii]!='\0'; ii++) 
  {
    unsigned char msg = rxbuffer[ii];
    
    
    switch(parserState) {  
      //START state
    case START: 
      //Check for a $ to move to next state
      switch (msg) {       
      case '$':    
        parserState = TALKER;         
        break;
      default:
        n=0;
        flag=0;
        cs=0;
        
        break;
      }
      break;
      
      //TALKER state, where we expect the ID and MSG
    case TALKER:     
      switch (msg) { 
        //If any of these characters, move back to START         
      case '*':      
      case '\r': 
      case '\n':           
        //We put this case in here to fallback if we recv GLL msg
      case 'L': 
        parserState = START; 
        //n=0;
        //flag=0;
        //cs=0;
        break; 
        //If we have a comma, don't do anything
      case ',':     
        flag = 1;
        cs ^= msg;
        break;    
      default:
        //If this is not a character OR we are over 6 characters, 
        //something went wrong
        if (n>5 || !isalpha(msg)) {
          parserState = START; 
        } else {
          //no need to store
          n++;
          cs ^= msg;
        }        
        break;   
      }
      //If we have the right num of chars (GPRMC) and a comma, move on
      if ((n==5) && flag) {
        parserState = UTC; 
        //Reset our counter for the next state
        n=0;
        flag=0;
      }
      break;
      
      //UTC state, where we expect time
    case UTC:
      //If this is a digit, save
      //Otherwise, go back (error)
      if( isdigit(msg) ) {
        utc[n++] = msg;
        cs ^= msg;
      } else if (msg == ',') {
        flag=1;
        cs ^= msg;
      } else {
        parserState=START;
        break;
      }
      
      //If we reach the maximum size, something went wrong
      //Else if we see a comma and we have the minimum number of chars,
      //move on...
      if (n==(UTC_SIZE)) {
        parserState=START;
        n=0;
        flag=0;        
      } else if ((n>=UTC_MIN) && flag) {
        //Convert to integer!
        //utc_i=atoi(utc);
        parserState=STATUS;
        n=0;
        flag=0;
      } else if (flag) {        
        parserState=START;
        n=0;
        flag=0;  
      }   
      break;
    case STATUS:
      //Do not need to store...just check we get the right thing
      if( msg=='A' ) {
        n++;
        cs ^= msg;
      } else if (msg == ',') {
        flag=1;
        cs ^= msg;
      } else {
        parserState=START;
        break;
      }  
      
      //We can move if we have 1 character and flag
      if (n>1) {
        parserState=START;
        n=0;
        flag=0;        
      } else if ( (n==1) && flag ) {
        parserState=LAT_DEG;
        n=0;
        flag=0;
      } else if (flag) {        
        parserState=START;
        n=0;
        flag=0;  
      }       
      break;   
      
      //LATITUDE state
    case LAT_DEG:
      //We only add to buffer if this is a digit
      if( isdigit(msg) ) {
        lat_deg[n++] = msg;
        cs ^= msg;
      } else {
        parserState=START;
        break;
      }   
      
      if (n==(LAT_DEG_SIZE)) {        
        parserState=START;
        n=0;
        flag=0;
      } else if ( (n==LAT_DEG_MIN) ) {
        lat_deg_i = atoi(lat_deg);
        parserState=LAT_MIN;
        n=0;
        flag=0;
      } else if (flag) {        
        parserState=START;
        n=0;
        flag=0;  
      }       
      break;  
      
      //LAT_MIN state
    case LAT_MIN        :
      //We only add to buffer if this is a digit
      if( isdigit(msg) || msg=='.') {
        lat_min[n++] = msg;
        cs ^= msg;
      } else if (msg == ',') {
        flag=1;
        cs ^= msg;
      } else {
        parserState=START;
        break;
      }  
      
      //We can move to LAT_DIR if we have at least 3 chars and a flag
      if (n==(LAT_MIN_SIZE)) {
        parserState=START;
        n=0;
        flag=0;        
      } else if ( (n>=LAT_MIN_MIN) && flag ) {
        lat_min_f = atof(lat_min);
        parserState=LAT_DIR;
        n=0;
        flag=0;
      } else if (flag) {        
        parserState=START;
        n=0;
        flag=0;  
      }      
      break;       
      
    //LATITUDE DIRECTION state
    case LAT_DIR:
      //We only add to buffer if this is a N or S
      if( msg=='N' || msg=='S' ) {
        lat_dir[n++] = msg;
        cs ^= msg;
      } else if (msg == ',') {
        flag=1;
        cs ^= msg;
      } else {
        parserState=START;
        break;
      }  
      
      //We can move to longitude if we have 1 character    
      if (n==(LAT_DIR_SIZE)) {
        parserState=START;
        n=0;
        flag=0;        
      } else if ( (n==LAT_DIR_MIN) && flag ) {
        parserState=LONG_DEG;
        n=0;
        flag=0;
      } else if (flag) {        
        parserState=START;
        n=0;
        flag=0;  
      }      
      break;       
      //LONGITUDE state
    case LONG_DEG:
      //We only add to buffer if this is a digit
      if( isdigit(msg)) {
        long_deg[n++] = msg;
        cs ^= msg;
      } else {
        parserState=START;
        break;
      }  
      //We can move to longitude if we have at least characters
       
      if (n==(LONG_DEG_SIZE)) {
        parserState=START;
        n=0;
        flag=0;        
      } else if ( (n==LONG_DEG_MIN) ) {
        long_deg_i = atoi(long_deg);
        parserState=LONGI_MIN;
        n=0;
        flag=0;
      } else if (flag) {        
        parserState=START;
        n=0;
        flag=0;  
      }      
      break;
      
    case LONGI_MIN        :
      //We only add to buffer if this is a digit
      if( isdigit(msg) || msg=='.') {
        long_min[n++] = msg;
        cs ^= msg;
      } else if (msg == ',') {
        flag=1;
        cs ^= msg;
      } else {
        parserState=START;
        break;
      }  
      
      //We can move to LAT_DIR if we have at least 3 chars and a flag
       
      if (n==(LONG_MIN_SIZE)) {
        parserState=START;
        n=0;
        flag=0;        
      } else if ( (n>=LONG_MIN_MIN) && flag ) {
        long_min_f=atof(long_min);
        parserState=LONG_DIR;
        n=0;
        flag=0;
      } else if (flag) {        
        parserState=START;
        n=0;
        flag=0;  
      }       
      break;     
      //LONGITUDE DIRECTION state
    case LONG_DIR:
      //We only add to buffer if this is a E or W
      if( msg=='E' || msg=='W' ) {
        long_dir[n++] = msg;
        cs ^= msg;
      } else if (msg == ',') {
        flag=1;
        cs ^= msg;
      } else {
        parserState=START;
        break;
      }  
      //We can move if we have 1 character
      if (n==(LONG_DIR_SIZE)) {
        parserState=START;
        n=0;
        flag=0;        
      } else if ( (n==LONG_DIR_MIN) && flag ) {
        parserState=SPEED;
        n=0;
        flag=0;
      } else if (flag) {        
        parserState=START;
        n=0;
        flag=0;  
      }       
      break;   
      
    case SPEED:
      //We only add to buffer if this is a digit
      if( isdigit(msg) || msg=='.') {
        speed[n++] = msg;
        cs ^= msg;
      } else if (msg == ',') {
        flag=1;
        cs ^= msg;
      } else {
        parserState=START;
        break;
      }  
      
      //We can move to if we have 4 characters
      
      if (n==(SPEED_SIZE)) {
        parserState=START;
        n=0;
        flag=0;        
      } else if ( (n>=SPEED_MIN) && flag) {
        speed_f = atof(speed);
        parserState=TRACK;
        n=0;
        flag=0;
      } else if (flag) {        
        parserState=START;
        n=0;
        flag=0;  
      }       
      break;
    case TRACK:
      //We only add to buffer if this is a digit
      if( isdigit(msg) || msg=='.') {
        track[n++] = msg;
        cs ^= msg;
      } else if (msg == ',') {
        flag=1;
        cs ^= msg;
      } else {
        parserState=START;
        break;
      }  
      
      //We can move to if we have 4 characters
      
      if (n==(TRACK_SIZE)) {
        parserState=START;
        n=0;
        flag=0;        
      } else if (( n>=TRACK_MIN ) && flag) {
        track_f = atof(track);
        parserState=DATE;
        n=0;
        flag=0;
      } else if (flag) {        
        parserState=START;
        n=0;
        flag=0;  
      }
      break;
    case DATE:
      //We only add to buffer if this is a digit
      if( isdigit(msg) ) {
        //date[n++] = msg;
        n++;
        cs ^= msg;
      } else if (msg == ',') {
        flag=1;
        cs ^= msg;
      } else {
        parserState=START;
        break;
      }  
      
      //We can move to if we have 6 characters
      if ( (n>=6) && flag ) {
        //strcat(date, "\n");
        parserState=MAG_VAR;
        n=0;
        flag=0;
      } else if (flag) {        
        parserState=START;
        n=0;
        flag=0;  
      }       
      break;
    case MAG_VAR:
      //We only add to buffer if this is a digit
      if( isdigit(msg) || msg=='.' ) {
        //don't copy, as we dont care about this
        n++;
        cs ^= msg;
      } else if (msg == ',') {
        flag=1;
        cs ^= msg;
      } else {
        parserState=START;
        break;
      }  
      
      //We can move to if we have 6 characters
      if ( (n>=1) && flag ) {
        //strcat(date, "\n");
        parserState=MAG_DIR;
        n=0;
        flag=0;
      } else if (flag) {        
        parserState=START;
        n=0;
        flag=0;  
      }       
      break;
      //LONGITUDE DIRECTION state
    case MAG_DIR:
      //We only add to buffer if this is a E or W
      if( msg=='E' || msg=='W' ) {        
        parserState=CHECKSUM1;
        n=0;
        flag=0;
        cs ^= msg;
      } else {
        parserState=START;        
        n=0;
        flag=0;
        break;
      }  
        
      break;   
    case CHECKSUM1:
      if( msg=='*' ) {
        parserState=CHECKSUM2;
        n=0;
        flag=0;
        break;
      } else {
        parserState=START;
        n=0;
        flag=0;
        break;
      }  
      break;
    case CHECKSUM2:
      switch (msg) { 
        //If any of these characters, move back to START                
      case '\r': 
        flag=1;
        break;
      case '\n':           
        flag=2;         
        break;   
      default:
        //If this is not a character OR we are over 6 characters, 
        //something went wrong
        if (n>2) {
          parserState = START; 
        } else {
          checksum2[n++] = msg;
        }        
        break;   
      }
      
      //Convert hex to int
      unsigned int number = (int)strtol(checksum2, NULL, 16);
      
      //If we go beyond 4 characters, something went wrong
      if ( (n==2) && (flag==2) && (number == cs)) {
        //Set Task to Run
        Run_TaskN(2);
        
        parserState=START;
        n=0;
        flag=0;
        cs=0;
        
        //Clear the buffer!
        memset(&rxbuffer[0], 0, sizeof(rxbuffer));  
      }       
      break;
      
    }        
  }
        
  //Clear the buffer!
  memset(&rxbuffer[0], 0, sizeof(rxbuffer)); 
  
}


         
         
         