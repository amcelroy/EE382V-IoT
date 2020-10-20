#include <msp430.h> 

#define READ_BUFFER_SIZE 32
#define BUFFER_SIZE 1024

#define LPM3_GIE() __bis_SR_register(LPM3_bits + GIE)

#define LPM0_GIE() __bis_SR_register(LPM0_bits + GIE)

const unsigned char AFI = 0x50; //Medical

unsigned short transmit_buffer[BUFFER_SIZE];

unsigned char request_transmit = 0;

unsigned short transmit_counter = 0;

unsigned short buffer_write_index = 0;

unsigned short buffer_read_index = 0;

unsigned char flag_inventory = 0;
unsigned char flag_select = 0;
unsigned char flag_address_mode = 0;
unsigned char host_uid[8];
const unsigned char info_flags = 0x6;
const unsigned char VICC_MAPPING_H = 0x0E;
const unsigned char VICC_MAPPING_L = 0x01;              //Block size of 2 bytes, 8 blocks

typedef enum TIMER_STATES_Enum{
    TRANSMIT,
    ACQUIRE,
} TIMER_STATES;


TIMER_STATES TIMER_STATE = ACQUIRE;

void configure_clocks(){
    //CCSCTL0
    CCSCTL0_H = 0xA5; // Unlocks the CCS registers

    //CCSCTL1
    //No change

    //CCSCTL4 Settings
    //All clocks set to LF-Oscillator
    //CCSCTL4 &= ~SELA0;

    //CCSCTL5 Settings
    CCSCTL5 &= ~DIVA__8;     //Clear DIVA0
    //CCSCTL5 |= DIVA__32;     //Set DIVA to 64kHz

    CCSCTL5 &= ~DIVS__8;     //Clear DIVA0
    //CCSCTL5 |= DIVS__32;     //Set DIVA to 64kHz

    CCSCTL5 &= ~DIVM__8;     //Clear DIVA0
    //CCSCTL5 |= DIVM__32;     //Set DIVA to 64kHz
    //Use the default values for S and M clock, they will be disabled later

    //CCSCTL6 Settings
    //XTOFF is disabled by default

    //CCSCTL7 Settings
    //No changes

    //CCSCTL8 Settings
    //No changes
}

void configure_SPI(){
    UCB0CTLW0 |= UCSWRST;

    //UCB0CTLW0 Settings
    UCB0CTLW0 |= UCMST;         //Set to master mode
    UCB0CTLW0 |= UCSSEL__ACLK;  //Set SPI to run off the ACLK
    UCB0CTLW0 |= UCMODE_0;      //Set to 3 wire spi
    UCB0BRW = 0xFE;
    //UCB0CTLW0 |= UCSTEM;        //Configure STE as chip select

    //UCB0BRW Settings
    //No changes

    //UCB0STATW Settings
    //No changes

    UCB0CTLW0 &= ~UCSWRST;                      //Clears the reset bit, SPI communication enabled

    UCB0IE |= UCRXIE;                           //Enable interrupts for RX
    //UCB0IE |= UCTXIE;
}

void configure_rfpmm(){
    //RFPMMCTL0 Settings
    //No changes

    //RFPMMCTL1 Settings
    //No changes
}

void configure_fram(){
    //FRCTL0 Settings
    //No Change

    //GCCTL0 Settings
    //No Change

    //GCCTL1 Settings
    //No Change
}

void configure_timer_a(){

    //TA0CTL = TAIE;

    //TA0CCTL Settings
    TA0CCTL0 = CCIE;
    TA0CCR0 = 64;

    //TA0CCTL Settings
    TA0CCTL1 = CCIE;
    TA0CCR1 = 1024;

    //TA0CTL Settings
    TA0CTL = TASSEL__ACLK | MC__CONTINUOUS | ID__1;     //Set timer from ACLK
}

void configure_rf(){
    //RF13MCTL Settings
    //No changes

    //RF13MINT Settings
    RF13MINT |= RX13MRFTOIE;    //Enable RF Timeout interrupt
    RF13MINT |= RX13MRXEIE;     //Enable RX error interupt
    RF13MINT |= RX13MOUFLIE;    //Enable overflow / underflow interrupt
    RF13MINT |= RF13MTXIE;      //TX done
    RF13MINT |= RF13MRXIE;      //RX done
    RF13MINT |= RF13MTXIFG;
    RF13MINT |= RF13MRXIFG;
    RF13MINT |= RF13MSLIFG;
    RF13MINT |= RF13MSLIE;
}

void configure_gpio(){
    //Configure all ports for default use (I/O), except 0 to 3, for SPI
    P1DIR |= BIT0;
    P1SEL1 &= ~BIT0;
    P1SEL0 |= BIT0;

    P1DIR |= BIT1;
    P1SEL1 &= ~BIT1;
    P1SEL0 |= BIT1;

    P1DIR |= BIT2;
    P1SEL1 &= ~BIT2;
    P1SEL0 |= BIT2;

    P1DIR |= BIT3;
    P1OUT |= BIT3;
    P1OUT |= BIT3;
}

/**
 * main.c
 */
int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer

    configure_clocks();
    configure_gpio();
    configure_timer_a();
    configure_SPI();
    //configure_rfpmm();
    //configure_fram();
    configure_rf();

    __bis_SR_register(LPM0_bits + GIE);

                                                        //Chip will be configured in LMP3 Mode, as the Timer used for initiating DAQ
    //LPM3_GIE();
}

__inline void parseFlag(unsigned char flag){
    flag_inventory = flag & 0x4;            //2nd bit
    if(!flag_inventory){
        flag_select = flag & 0x10;          //5th bit
        flag_address_mode = flag & 0x20;    //6th bit
    }
}

__inline sendUUID(){
    RF13MTXF_L = 0x01;
    RF13MTXF_L = 0x02;
    RF13MTXF_L = 0x03;
    RF13MTXF_L = 0x04;
    RF13MTXF_L = 0x05;
    RF13MTXF_L = 0x23;
    RF13MTXF_L = 0x02;  //TI
    RF13MTXF_L = 0xE0;
}

__inline unsigned char checkUUID(){
    unsigned char valid_address = 1;
    valid_address &= ((&RF13MRXBUF_L)[2] == 0x01);
    valid_address &= ((&RF13MRXBUF_L)[3] == 0x02);
    valid_address &= ((&RF13MRXBUF_L)[4] == 0x03);
    valid_address &= ((&RF13MRXBUF_L)[5] == 0x04);
    valid_address &= ((&RF13MRXBUF_L)[6] == 0x05);
    valid_address &= ((&RF13MRXBUF_L)[7] == 0x23);
    valid_address &= ((&RF13MRXBUF_L)[8] == 0x02);
    valid_address &= ((&RF13MRXBUF_L)[9] == 0xE0);
    return valid_address;
}

__inline void parse_iso_15693(){
    char x = 0;

    parseFlag((&RF13MRXBUF_L)[0]);

    unsigned char valid_address = 0;
    if(flag_address_mode){
        valid_address = checkUUID();
    }

    switch((&RF13MRXBUF_L)[1]){
    case 0x01:
        //Inventory
        RF13MTXF_L = 0x08; //No errors
        RF13MTXF_L = 0x00; //No DSFID
        sendUUID();
        break;

    case 0x2B:
        RF13MTXF_L = 0x08; //High bandwidth
        RF13MTXF_L = info_flags;
        sendUUID();
        RF13MTXF_L = AFI;
        RF13MTXF_L = VICC_MAPPING_L;
        RF13MTXF_L = VICC_MAPPING_H;
        break;

    case 0x20:
        if(valid_address){
            RF13MTXF_L = 0x08; //High bandwidth
            unsigned short checksum = 0;
            for(x = 0; x < READ_BUFFER_SIZE; x++){
                if(buffer_read_index + x >= BUFFER_SIZE){
                    buffer_read_index = 0;
                }
                RF13MTXF = transmit_buffer[x + buffer_read_index];
                checksum += transmit_buffer[x + buffer_read_index];
                buffer_read_index += 1;
            }
            RF13MTXF = checksum;
        }
        break;

    default:
        x = 1;
        break;
    }

    RF13MCTL |= RF13MTXEN;
}

#pragma vector=RF13M_VECTOR
__interrupt void RF_Int(void){
    switch(RF13MIV_L){
    case 2:
        //Data recieved
        request_transmit = 1;
        break;
    case 4:
        //Transmit done
        RF13MCTL |= RF13MRXEN;
        break;
    default:
        break;
    }
}

#pragma vector=TIMER0_A1_VECTOR
__interrupt void TIMER0_A1_ISR(void){
    //Fire off the Acquire signal every 125Hz

    unsigned int iv = TA0IV;

    TA0CCR1 += 160;

    if(request_transmit && buffer_write_index != 0){
        TIMER_STATE = TRANSMIT;
    }

    switch(iv){
    case 0x2:
        switch(TIMER_STATE){
        case ACQUIRE:
            transmit_buffer[buffer_write_index] = buffer_write_index;
            buffer_write_index += 1;
            if(buffer_write_index >= BUFFER_SIZE){
                buffer_write_index = 0;
            }
            break;
        case TRANSMIT:
            parse_iso_15693();
            TIMER_STATE = ACQUIRE;
            request_transmit = 0;
            break;
        }
//        timer_loop_counter += 1;
//        if(timer_loop_counter >= 16){
//            timer_loop_counter = 0;
//        }
        break;
    }
}


#pragma vector=TIMER0_A0_VECTOR
__interrupt void TIMER0_A0_ISR(void){

}

