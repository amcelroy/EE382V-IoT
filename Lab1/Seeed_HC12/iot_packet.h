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

#define SOURCE_ID 1 // source address

// size of data allowed in a single packet
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
#define AJP_GOODBYE         0xA0
#define AJP_ACKNOWLEDGE     0x01
#define AJP_ERROR  	        0x02

#define RESPONSE_TIMEOUT 1000 // 1 second

#define HEADER_SIZE 6 //Everything except data and internal byte counter

typedef void (*packet_received_success)(struct IOT_PACKET*);
typedef void (*packet_received_error)(struct IOT_PACKET*);



struct IOT_PACKET {
	uint8_t destination; // destination address
	uint8_t source;      // source address
	uint8_t status;
	uint8_t packet_number;
	uint8_t total_packets;
	uint8_t data_length; // currently max of data size, allows for data
	                     // less than DATA_SIZE
	// DATA_SIZE bounded by 255 b/c of data_length precision
	uint8_t data[DATA_SIZE]; 
	uint16_t checksum;

	uint16_t internal_byte_counter;

    packet_received_success success_callback;
    packet_received_error error_callback;
};

struct IOT_TRANSACTION {
    struct IOT_PACKET * packet;      // outbox
    struct IOT_PACKET * packet_recv; // inbox
    uint8_t destination; // intended recipient
    uint8_t packetsReceived;
    uint8_t size; // size in number of packets. Equal to total_packets
    bool isTransmitter;
};

// begin transaction
// start order
// send hello
// wait for acknowledgement
// return 0 if sucessful, 1 if timeout

struct IOT_TRANSACTION * ajp_beginTransmittal(uint8_t destination, int totalSize);

int ajp_endTransmittal(struct IOT_TRANSACTION * t);


void ajp_clear_packet(struct IOT_PACKET *p);

void ajp_reset_packet_counter(struct IOT_PACKET *p);

uint8_t ajp_tx_get_byte(struct IOT_PACKET *p);

/** ajp_rx_add_byte
 *  Reconstructs a packet on the receiving end.
 *
 */
void ajp_rx_add_byte(struct IOT_PACKET *p, uint8_t byte);
unsigned short ajp_checksum(struct IOT_PACKET *p, bool store);

uint16_t ajp_sizeof_packet();

void ajp_write(struct IOT_PACKET *p, uint8_t * data, uint32_t dataLength);



// listen for incoming packets.
// upon receipt of hello, send acknowledgement
// upon receipt of anything else, send reset/error message
struct IOT_TRANSACTION * ajp_listen();

// sends a single byte as data. The protocol should take care of the rest.
void ajp_sendByte(struct IOT_TRANSACTION * t, char datum);

int ajp_receiveByte(struct IOT_TRANSACTION *t, uint8_t * datum);

void ajp_acknowledge(struct IOT_TRANSACTION * t);

#endif
