#include "iot_packet.h"

uint8_t checksum_buffer[sizeof(uint16_t)];

void ajp_clear_packet(struct IOT_PACKET *p){
	p->checksum = 0;
	p->packet_number = 0;
	p->total_packets = 0;
	p->status = 0;
	p->address = 0;
	memset(p->data, 0, DATA_SIZE);
    p->success_callback = 0;
    p->error_callback = 0;
    p->internal_byte_counter = 0;
}

uint16_t ajp_sizeof_packet(){
	return DATA_SIZE + HEADER_SIZE;
}

void ajp_reset_packet_counter(struct IOT_PACKET *p){
    p->internal_byte_counter = 0;
}

// build packet on transmit send
uint8_t ajp_get_byte(struct IOT_PACKET *p){
	uint8_t retval = 0;
		
	switch(p->internal_byte_counter){
	case 0:
		memcpy(checksum_buffer, &p->checksum, sizeof(uint16_t));
		retval = checksum_buffer[0];
		break;

	case 1:
		retval = checksum_buffer[1];
		break;

	case 2:
		retval = p->status;
		break;

	case 3:
		retval = p->address;
		break;

	case 4:
		retval = p->packet_number;
		break;
	
	case 5:
		retval = p->total_packets;
		break;

	default:
		retval = p->data[p->internal_byte_counter - 6]; //6 is checksum + status + address + packet_number + total_packets in bytes
		if(p->internal_byte_counter  - HEADER_SIZE >= DATA_SIZE){
			return 0;
		}
		break;
	}

	p->internal_byte_counter += 1;
	return retval;
}

// reconstruct packet on receiving end
void ajp_add_byte(struct IOT_PACKET *p, uint8_t byte){
	uint8_t retval = 0;
		
	switch(p->internal_byte_counter){
	case 0:
		checksum_buffer[0] = byte;
		break;

	case 1:
		checksum_buffer[1] = byte;
		memcpy(&p->checksum, checksum_buffer, sizeof(uint16_t));
		break;

	case 2:
		p->status = byte;
		break;

	case 3:
		p->address = byte;
		break;

	case 4:
		p->packet_number = byte;
		break;
	
	case 5:
		p->total_packets = byte;
		break;

	default:
		p->data[p->internal_byte_counter - HEADER_SIZE] = byte; //6 is checksum + status + address + packet_number + total_packets in bytes
		break;
	}

    p->internal_byte_counter += 1;

    if(p->internal_byte_counter - HEADER_SIZE >= DATA_SIZE){
        //run the checksum
        uint16_t checksum = ajp_checksum(p, false);
        if(checksum == p->checksum){
            if(p->success_callback){
                p->success_callback(p);
            }
        }else{
            if(p->error_callback){
                p->error_callback(p);
            }
        }
    }
}

unsigned short ajp_checksum(struct IOT_PACKET *p, bool store){
	uint16_t sum = 0;
    uint8_t *ptr = (uint8_t*)p;
	for(int i = 2; i < ajp_sizeof_packet(); i++){
        sum += (unsigned short)ptr[i];
	}

	if(store){
		p->checksum = sum;
	}

	return sum;
}


// begin transaction
// start order
// send hello
// wait for acknowledgement
// return 0 if sucessful, 1 if timeout
int ajp_beginTransaction(int totalPackets)
{
// 	Int retval; 
// 	//struct IOT_PACKET pack;
// 	//Struct IOT_PACKET * p = &pack; 
// 	HC12_ReadAllInput();
// //ajp_clear_packet(p);
// 	// send hello
// UART1_OutChar(AJP_HELLO); 
// 	// wait for an acknowledge for some time
// Char in;
// Count responseWait = 0;  
// // delay 10 ms
// In = UART1_InCharNonBlock()
// while(in == 0 && responseWait < RESPONSE_TIMEOUT){
// 	// delay 10 ms
// 	responseWait += 10;
// In = UART1_InCharNonBlock();
// } 

// // evaluate listening
// if(in == 0 || responseWait >= RESPONSE_TIMEOUT){
// 	UART1_OutChar(AJP_ERROR);
// 	retval = -1; 
// }
// else if(in == AJP_HELLO){
// 	UART1_OutChar(AJP_ACKNOWLEDGE);
// 	retval = 0; 
// } 
// Else {
// 	// left separate for potentially other types of error
// 	UART1_OutChar(AJP_ERROR);
// 	retval = -2; 
// }

// Return retval; 
	
}


