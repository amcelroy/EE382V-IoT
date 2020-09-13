#ifndef __IOT_PACKET_H__
#define __IOT_PACKET_H__

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#ifdef __TI_COMPILER_VERSION__
#include "../inc/SysTickInts.h"
#include "../inc/Clock.h"
#include "../inc/UART1.h"
#endif

#define DATA_SIZE 64

// status byte:
// 7: Send (1) / Receive(0)
// 6: Hello (1)
// 5: Goodbye (1)
// 4: 
// 3: 
// 2:
// 1: ERROR (1) / NoError (0)
// 0: acknowledge (1) 

#define AJP_HELLO		    0xC0
#define AJP_GOODBYE           0xA0
#define AJP_ACKNOWLEDGE 0x01
#define AJP_ERROR  	    0x02

#define RESPONSE_TIMEOUT 1000 // 1 second

#define HEADER_SIZE 6 //Everything except data and internal byte counter

typedef void (*packet_received_success)(struct IOT_PACKET*);

typedef void (*packet_received_error)(struct IOT_PACKET*);

struct IOT_PACKET {
	uint16_t checksum;
	uint8_t status; 
	uint8_t address; 
	uint8_t packet_number;
	uint8_t total_packets;
	uint8_t data[DATA_SIZE]; 
	uint16_t internal_byte_counter;
    packet_received_success success_callback;
    packet_received_error error_callback;
};



// begin transaction
// start order
// send hello
// wait for acknowledgement
// return 0 if sucessful, 1 if timeout
int ajp_beginTransaction(int totalPackets);

void ajp_clear_packet(struct IOT_PACKET *p);

void ajp_reset_packet_counter(struct IOT_PACKET *p);

uint8_t ajp_get_byte(struct IOT_PACKET *p);

void ajp_add_byte(struct IOT_PACKET *p, uint8_t byte);
unsigned short ajp_checksum(struct IOT_PACKET *p, bool store);

uint16_t ajp_sizeof_packet();

void ajp_write(struct IOT_PACKET *p, uint8_t * data, uint32_t dataLength ); 

// end transaction
// end order
// send goodbye
// wait for acknowledgement
void ajp_endTransaction();



// listen for incoming packets.
// upon receipt of hello, send acknowledgement
// upon receipt of anything else, send reset/error message
void ajp_receive(); 


#endif
