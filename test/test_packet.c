#include <stdio.h>
#include "iot_packet.h"

void p2_success(struct IOT_PACKET *p){
    printf("Success!\r\n");
}

void p2_error(struct IOT_PACKET *p){
    printf("Error :(");
}

int main(int argc, char **argv){

    struct IOT_PACKET p1;
    struct IOT_PACKET p2;

    ajp_clear_packet(&p1);
    ajp_clear_packet(&p2);

    p2.success_callback = p2_success;
    p2.error_callback = p2_error;

    p1.address = 1;
    p1.packet_number = 1;
    p1.total_packets = 1;
    p1.status = AJP_HELLO;
    uint16_t crc_pre = ajp_checksum(&p1, true);
    memcpy(p1.data, "Hello World!", strlen("Hello World!"));
    uint16_t crc_post = ajp_checksum(&p1, true);

    uint8_t tx_channel[ajp_sizeof_packet()]; //Demo transmission channel

    for(int i = 0; i < ajp_sizeof_packet(); i++){
        uint8_t packet_byte = ajp_get_byte(&p1);
        tx_channel[i] = packet_byte;
    }

    for(int i = 0; i < ajp_sizeof_packet(); i++){
        ajp_add_byte(&p2, tx_channel[i]);
    }

    if(ajp_checksum(&p2, false) == ajp_checksum(&p1, false)){
        printf("Checksums is the same for p1 and p2 \r\n");
    }
    //p1 and p2 should be the same here
    
    
    
    return 0;
}