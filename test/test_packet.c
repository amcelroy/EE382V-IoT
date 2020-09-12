#include "../include/iot_packet.h"

void IoT_Packet_Complete_Callback(struct IoT_PACKET *packet){
    printf("Packet Complete");
}

int main(int argc, char **argv){

    struct IoT_PACKET p1;
    struct IoT_PACKET p2;

    IoT_Packet_Reset(&p2);
    IoT_Packet_Reset(&p1);

    memcpy(p1.header, "command", strlen("command"));
    memcpy(p1.data, "Hello World!", strlen("Hello World!"));
    p1.total_packets = 1;
    p1.current_packet = 1;
    
    IoT_Packet_Generate_Checksum(&p1, 1);

    for(int i = 0; i < IoT_Packet_Sizeof(&p1); i++){
        //Send data here
        uint8_t pbyte = IoT_Packet_Get_Byte(&p1);

        //get data here
        enum  IoT_PACKET_ADD_BYTE_CODES retval = IoT_Packet_Add_Byte(&p2, pbyte);

        if(retval == IoT_PACKET_PARSING){
            //keep going
        }else{
            printf("Warning!");
        }
    }

    //p1 and p2 should be the same here
    
    
    
    return 0;
}