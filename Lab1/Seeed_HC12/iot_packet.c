#include "iot_packet.h"



// private global variables

static struct IOT_TRANSACTION Transaction;
static struct IOT_PACKET Packet;
static struct IOT_PACKET Packet_Receive;
static bool validPacket = false;
// private function declaration

// packet helper functions
static void ajp_sendPacket(struct IOT_PACKET *p);
static void ajp_clearPacket(struct IOT_PACKET *p);
static void ajp_incrementPacket(struct IOT_PACKET *p);
static int  ajp_evaluateIncomingPacket(void);
static void ajp_readAllInput(void);

// packet waiting
static int ajp_waitForAcknowledgment(struct IOT_TRANSACTION * t);
static int ajp_waitForIncomingPacket(bool timeout, int timeout_val);


uint8_t checksum_buffer[sizeof(uint16_t)];



uint16_t ajp_sizeof_packet(){
	return DATA_SIZE + HEADER_SIZE;
}

void ajp_reset_packet_counter(struct IOT_PACKET *p){
    p->internal_byte_counter = 0;
}

// build packet on transmit send
uint8_t ajp_tx_get_byte(struct IOT_PACKET *p){
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
		retval = p->destination;
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
void ajp_rx_add_byte(struct IOT_PACKET *p, uint8_t byte){
	//uint8_t retval = 0;
		
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
		p->destination = byte;
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

// create a sum complement checksum:
// basically add all words (defined here as uint8_t) and store
// the twos complement.
// receiver should add all numbers including the checksum and
// get 0.
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

// total size can be 0 for a stream!
struct IOT_TRANSACTION * AJP_beginTransmittal(uint8_t destination, int totalSize)
{
  // clear all that input good good
  ajp_readAllInput();

  struct IOT_TRANSACTION * t = &Transaction;
  t->isTransmitter = true;
  t->destination = destination;
  // currently sets the transaction packet to the global packet
  t->packet = &Packet;
  t->packet_recv = &Packet_Receive;

  // for clear coding.
  struct IOT_PACKET * p = t->packet;

  ajp_clearPacket(p);

  // calculate number of packets necessary
  if(totalSize == 0)
  {
      // optional streaming (total packets unknown)
      p->total_packets = 0xFF;
  }
  else
  {
      uint8_t numPackets = totalSize / DATA_SIZE;
      if(totalSize % DATA_SIZE != 0){
          // there will be one packet with data smaller than
          // the total data size.
          numPackets++;
      }
      p->total_packets = numPackets;
  }

  t->size = p->total_packets;
  // add addresses
  p->destination = destination;
  p->source      = SOURCE_ID; // should be hardcoded



  // send hello
  p->status = AJP_HELLO;
  // should only send header and checksum
  ajp_sendPacket(p);

  // get ready to send the next packet
  ajp_incrementPacket(p);

  // wait for an acknowledge for some time
  if(ajp_waitForAcknowledgment(t) == 0){
      return t;
  }
  else{
      return 0;
  }
}

int AJP_endTransmittal(struct IOT_TRANSACTION * t)
{
    // send any data remaining
    struct IOT_PACKET * p = t->packet;
    if(p->data_length > 0){
        ajp_sendPacket(p);
    }

    // wait for acknowledge
    if(!ajp_waitForAcknowledgment(t) == 0){
        // no acknowledge
        while(1);
    }

    // deconstruct the transaction
    ajp_clearPacket(p);
    t->packet = 0;
    t->packet_recv = 0;
    t->size = 0;
    return 0;
}

void AJP_sendByte(struct IOT_TRANSACTION * t, char datum)
{
    struct IOT_PACKET * p = t->packet;
    if(p != NULL  && t->isTransmitter){
        // a single byte has been added to the packet.
        p->data[p->data_length] = datum;
        p->data_length++;

        if(p->data_length == DATA_SIZE)
        {
            // packet full
            ajp_sendPacket(p);
            // go to the next packet
            ajp_incrementPacket(p);
        }
    }
    else {
        // error, no packet in the transaction
        // currently error handling is not implemented
        while(1);
    }
}

struct IOT_TRANSACTION * AJP_listen(void)
{
    ajp_readAllInput();

    struct IOT_TRANSACTION * t = &Transaction;
    t->isTransmitter = false;
    t->destination = 0; // currently unknown
    // currently sets the transaction packet to the global packet
    t->packet = &Packet;
    t->packet_recv = &Packet_Receive;


    ajp_waitForIncomingPacket(false, 0);
    // listens for a hello from anyone
    struct IOT_PACKET * p_rcv = t->packet_recv;
    struct IOT_PACKET * p = t->packet;
    t->destination = p_rcv->source;
    t->size = p_rcv->total_packets;

    ajp_clearPacket(p);

    p->destination = t->destination;
    p->source = SOURCE_ID;
    p->status = AJP_ACKNOWLEDGE;

    // returns an acknowledgement
    ajp_sendPacket(p);

    // invalidates current packet to receive new one.
    validPacket = false;

    return t;
}

int AJP_receiveByte(struct IOT_TRANSACTION *t, uint8_t * datum)
{
    // since everything is sequential, we can simply read through
    // the packet if it's valid.
    static int dataReadCount = 0;
    int retval;

    if(t->packetsReceived == t->size && dataReadCount == t->packet_recv->data_length){
        // no more data to read. Finished.
        dataReadCount = 0;
        validPacket = false;
        retval = 1;
    } else {
        while(validPacket != true || dataReadCount >= t->packet_recv->data_length)
        {
            validPacket = false;
            dataReadCount = 0;
            // if incoming packet is valid, this can set validPacket true
            ajp_waitForIncomingPacket(false, 0);
            t->packetsReceived++;
        }

        * datum = t->packet_recv->data[dataReadCount];
        dataReadCount++;
        retval = 0;
    }

    return retval;
}


void AJP_acknowledge(struct IOT_TRANSACTION * t)
{
    struct IOT_PACKET * p = t->packet;
    ajp_clearPacket(p);

    p->destination = t->destination;
    p->source = SOURCE_ID;
    p->status = AJP_ACKNOWLEDGE;

    // returns an acknowledgement
    ajp_sendPacket(p);
}

/*
 * PRIVATE FUNCTIONS
 * */

static void ajp_sendPacket(struct IOT_PACKET *p)
{
    // must calculate checksum as sent
    p->checksum = 0;

    uint8_t * sendptr = (uint8_t *) p;
    int i;
    // send header: 6 header values
    for(i = 0; i < 6; i++)
    {
        p->checksum += *sendptr;
        UART1_OutChar(*sendptr);
        sendptr++;
    }

    // send data
    for(i = 0; i < p->data_length; i++)
    {
        p->checksum += *sendptr;
        UART1_OutChar(*sendptr);
        sendptr++;
    }

    // two's complement for the checksum
    p->checksum = (p->checksum ^ 0xFF) + 1;
    // send checksum (length of 2 words)
    /* Old way of sending 16-bit checksum
    sendptr = (uint8_t*) &(p->checksum);
    for(i = 0; i < 2; i++)
    {
        UART1_OutChar(*sendptr);
        sendptr++;
    }
    */
    UART1_OutChar(p->checksum);

}

static void ajp_clearPacket(struct IOT_PACKET *p){
    p->checksum = 0;
    p->packet_number = 0;
    p->total_packets = 0;
    p->status = 0;
    p->destination = 0;
    p->source = 0;
    memset(p->data, 0, DATA_SIZE);
    p->success_callback = 0;
    p->error_callback = 0;
    p->internal_byte_counter = 0;
}

static void ajp_incrementPacket(struct IOT_PACKET *p)
{
    // increments packet to next value.
    // Resets packet-specific values
    p->packet_number++;
    p->data_length = 0;
    memset(p->data, 0, DATA_SIZE);
}

void ajp_readAllInput(void){uint8_t in;
// flush receiver buffer
  in = UART1_InCharNonBlock();
  while(in){
    in = UART1_InCharNonBlock();
  }
}

static int ajp_waitForAcknowledgment(struct IOT_TRANSACTION * t)
{
    int retval;
    // for now, doing away with timeout
    if(!ajp_waitForIncomingPacket(false, 0) == 0)
    {
        // here we have an invalid packet or it timed out.
        while(1);
    }

    struct IOT_PACKET * p_recv = t->packet_recv;

    if(p_recv->status == AJP_ACKNOWLEDGE && p_recv->source == t->destination)
    {
      retval = 0;
    } else {
      // receive error
      retval = -1;
    }

    //}
    //youHaveMail = 0; // I've read the mail value
    return retval;
}

static int ajp_waitForIncomingPacket(bool timeout, int timeout_val)
{
    char in;
    if(timeout){
        int time = 0;
        in = UART1_InCharNonBlock();
        while(in != SOURCE_ID && time < timeout_val){
            if(in == 0){
                // nothing in the fifo, so we wait
                Clock_Delay1ms(1);
                time++;
            }
            in = UART1_InCharNonBlock();
        }
        if(time >= timeout_val){
            return -2;
        }
    }
    else {
        in = UART1_InChar();
        while(in != SOURCE_ID){
            in = UART1_InChar();
        }
    }
    // once a message addressed to us is identified
    return ajp_evaluateIncomingPacket();
}

static int ajp_evaluateIncomingPacket(void)
{
    // if we've made it to here, we know that the
    // destination address for a message is us.
    struct IOT_PACKET * p = &Packet_Receive;
    uint8_t checksum = 0;
    int i;
    uint8_t * ptr;
    p->destination = SOURCE_ID;
    checksum += SOURCE_ID;
    // get the header
    for(ptr = &(p->source); (ptr < &p->data_length); ptr++){
        *ptr = UART1_InChar();
        checksum += *ptr;
    }

    if(p->data_length <= DATA_SIZE){
        ptr = &(p->data[0]);
        for(i = 0; i < p->data_length; i++){
            *ptr = UART1_InChar();
            checksum += *ptr;
            ptr++;
        }
    }

    // construct the 16 bit checksum.
    // switching to 8 - bit checksum
    // msb sent first. Changed for some reason?
    // p->checksum = UART1_InChar() << 8;
    p->checksum += UART1_InChar();

    // check for message validity
    if(checksum + p->checksum == 0){
        // this packet is valid
        validPacket = true;
        return 0;
    }

    else{
        // you have an error in the packet sent.
        validPacket = false;
        return -1;
    }
}
