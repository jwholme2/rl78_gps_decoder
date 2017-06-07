#include "gpssim.h"
#include "r_cg_macrodriver.h"
#include "r_cg_port.h"
#include "r_cg_serial.h"
#include "r_cg_adc.h"
#include "lcd.h"
#include "stdio.h"
#include "string.h"
#include "queues.h"
#include "r_cg_userdefine.h"

extern volatile uint8_t G_UART_SendingData, G_UART_ReceivingData;

//This is used for storing our GPRMC message
char msg[128];
//
////This is used to store the string from the rx_buffer
//char rxbuffer[RXBUFFER_SIZE];
////char txbuffer[128];
//
////Function that retrieves a string from the rx buffer
//void get_string(void) {  
//  //As long as there is data...
//  while (!Q_Empty(&rx_q)) {
//     unsigned char ret =  Q_Dequeue(&rx_q);
//     strncat(rxbuffer,  &ret,1);
//     
//     //Ensure that we always have a null-terminator
//     rxbuffer[RXBUFFER_SIZE-1]='\0'
//  }
//}
//
////Function that places a char string into tx buffer
//void send_string(char * buffer) {
//  //Continue as long as the queue is not full
//  //Or we have a message terminator \n
//  for (int i=0; buffer[i]!='\0'; i++) {
//    Q_Enqueue(&tx_q, buffer[i]);
//  }
//}


uint8_t checksum(char *s) {
    uint8_t c = 0;

    while(*s)
        c ^= *s++;

    return c;
}

void create_GLL_msg(char * buffer, int lat_deg, float lat_min, int lon_deg, float lon_min, unsigned
hr, unsigned min, unsigned sec) {
	uint8_t cs=0;
	char end[8];

	// form message
	sprintf(buffer, "$GPGLL,%02d%06.3f,N,%03d%06.3f,W,%02d%02d%02d,A,*", lat_deg, lat_min, lon_deg, lon_min,
	hr, min, sec);

	// form checksum
	cs = checksum(buffer);

	// undo effects of exoring in start $ and end *
	cs ^= '$' ^ '*';

	sprintf(end, "%02X\n\r", cs);
	strcat(buffer, end);
}

void create_RMC_msg(char * buffer, int lat_deg, float lat_min, int lon_deg, float lon_min, unsigned
hr, unsigned min, unsigned sec, float speed, float track, uint32_t date, float var) {
	uint8_t cs=0;
	char end[8];

	// form message
	//sprintf(buffer, "$GPRMC,%02d%02d%02d,A,%02d%06.5f,N,%03d%06.3f,W,%01.0f,%09.1f,%06ld,%05.1f,W*",
	//				hr, min, sec, lat_deg, lat_min, lon_deg, lon_min, speed, track, date, var);

	sprintf(buffer, "$GPRMC,%02d%02d%02d,A,%02d%06.3f,S,%03d%06.3f,E,%09.1f,%04.1f,%06ld,%05.1f,W*",
					hr, min, sec, lat_deg, lat_min, lon_deg, lon_min, speed, track, date, var);

        // form checksum
	cs = checksum(buffer);

	// undo effects of exoring in start $ and end *
	cs ^= '$' ^ '*';


	sprintf(end, "%02X\n\r", cs);
	strcat(buffer, end);
}


void inc_time(unsigned * hours, unsigned * minutes, unsigned * seconds) {
	(*seconds)++;
	if (*seconds > 59) {
		*seconds -= 60;
		(*minutes)++;
		if (*minutes > 59) {
			*minutes -= 60;
			(*hours)++;
		}
	}
}

void sim_motion(void) {
	static int16_t i, lat_deg = 35, lon_deg = 78;
	static uint32_t date=22213;
	static float lat_min = 11.111111, lon_min = 2.6, lat_step=0.011, lon_step=0.025;
	static float spd=1.2, trk=180.0, var=0.0;

	static unsigned h=0, m=0, s=0, ctr=0;
        
        static unsigned char bit = 0;

	//LCDPrintf(1,0, "GPSSimulator");
	//LCDPrintf(2,0, "deg minutes");

	//LCDPrintf(3,0, "%03d %06.3f", lat_deg, lat_min);
	//LCDPrintf(4,0, "%03d %06.3f", lon_deg, lon_min);
	//LCDPrintf(5,0, "%02d:%02d:%02d", h, m, s);
	//LCDPrintf(6,0, "%4d msgs", ctr);

        if (bit==1 ) {
        create_RMC_msg(msg, lat_deg, lat_min, lon_deg, lon_min, h, m, s, spd, trk, date, var);
        //         create_GLL_msg(msg, lat_deg, lat_min, lon_deg, lon_min, h, m, s);
        bit =0;
        } else {
          create_GLL_msg(msg, lat_deg, lat_min, lon_deg, lon_min, h, m, s);
          //        create_RMC_msg(msg, lat_deg, lat_min, lon_deg, lon_min, h, m, s, spd, trk, date, var);
          bit=1;
        }
        
	//create_GLL_msg(msg, lat_deg, lat_min, lon_deg, lon_min, h, m, s);
	//G_UART_SendingData = 1;
	//R_UART1_Send((uint8_t *) msg, strlen(msg));

        //Call send string!
        send_string(msg);
        
	// update variables and LCD
	lat_min += lat_step;
	lon_min += lon_step;
	ctr += 1;
	inc_time(&h, &m, &s);


#if 0	// delay between sentences unless user is pressing switch 3
	if (SW_3 == 1) {
		for (dly=0; dly<600000; dly++) {
			if (SW_3 == 0) // abandon delay immediately
				break;
			;
		}
	}
#endif

  if (++i >= NUM_STEPS) {
		lat_step *= -1.0;
		lon_step *= -1.0;
    i = 0;
	}
}

//void sim_motionGLL(void) {
//	static int16_t i, lat_deg = 35, lon_deg = 78;
//	static uint32_t date=22213;
//	static float lat_min = 11.111111, lon_min = 2.6, lat_step=0.011, lon_step=0.025;
//	static float spd=1.2, trk=180.0, var=0.0;
//
//	static unsigned h=0, m=0, s=0, ctr=0;
//
//	//LCDPrintf(1,0, "GPSSimulator");
//	//LCDPrintf(2,0, "deg minutes");
//
//	//LCDPrintf(3,0, "%03d %06.3f", lat_deg, lat_min);
//	//LCDPrintf(4,0, "%03d %06.3f", lon_deg, lon_min);
//	//LCDPrintf(5,0, "%02d:%02d:%02d", h, m, s);
//	//LCDPrintf(6,0, "%4d msgs", ctr);
//
// 
//        //create_RMC_msg(msg, lat_deg, lat_min, lon_deg, lon_min, h, m, s, spd, trk, date, var);
//        
//	create_GLL_msg(msg, lat_deg, lat_min, lon_deg, lon_min, h, m, s);
//	//G_UART_SendingData = 1;
//	//R_UART1_Send((uint8_t *) msg, strlen(msg));
//
//        //Call send string!
//        send_string(msg);
//        
//	// update variables and LCD
//	lat_min += lat_step;
//	lon_min += lon_step;
//	ctr += 1;
//	inc_time(&h, &m, &s);
//
//
//#if 0	// delay between sentences unless user is pressing switch 3
//	if (SW_3 == 1) {
//		for (dly=0; dly<600000; dly++) {
//			if (SW_3 == 0) // abandon delay immediately
//				break;
//			;
//		}
//	}
//#endif
//
//  if (++i >= NUM_STEPS) {
//		lat_step *= -1.0;
//		lon_step *= -1.0;
//    i = 0;
//	}
//}
