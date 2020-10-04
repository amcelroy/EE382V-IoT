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

/**
 * @brief Struct that stores all of the packet memory in 
 * contiguous memory.
 * 
 */

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

/**
 * @brief Resets memory, counters, and all other
 * packet information.
 * 
 * @param p 
 */
void ajp_clear_packet(struct IOT_PACKET *p);

/**
 * @brief Resets the internal packet counter associated
 * with adding and getting bytes.
 * 
 * @param p 
 */
void ajp_reset_packet_counter(struct IOT_PACKET *p);

/**
 * @brief Gets a byte to the packet and internally
 * updates the internal byte counter.
 * 
 * @param p 
 * @return uint8_t 
 */
uint8_t ajp_get_byte(struct IOT_PACKET *p);

/**
 * @brief Adds a byte to the packet and internally
 * updates the internal byte counter.
 * 
 * @param p 
 * @param byte 
 */
void ajp_add_byte(struct IOT_PACKET *p, uint8_t byte);

/**
 * @brief Performs a checksum on: status, address, packet_number, 
 * total_packets, and the data array.
 * 
 * If store is true, the checksum is stored in the checksum location.
 * 
 * @param p 
 * @param store Store the value or just compute it
 * @return unsigned short Checksum 
 */
unsigned short ajp_checksum(struct IOT_PACKET *p, bool store);

/**
 * @brief Gets the size of the packet, including: checksum, status, 
 * address, packet_number, total_packets, and the data array.
 * 
 * @return uint16_t Size of the packet
 */
uint16_t ajp_sizeof_packet();

void ajp_write(struct IOT_PACKET *p, uint8_t * data, uint32_t dataLength ); 

/**
 * @brief Ends the packet transaction and waits fo acknowledgement
 * 
 */
void ajp_endTransaction();

/**
 * @brief Listens for incoming packets.
 * 
 */
void ajp_receive(); 

// begin transaction
// start order
// send hello
// wait for acknowledgement
// return 0 if sucessful, 1 if timeout
int ajp_beginTransaction(int totalPackets);


#endif
