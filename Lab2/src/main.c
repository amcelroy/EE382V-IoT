#include "lab2.h"

void main(int argc, char **argv){

    struct device *gpio = device_get_binding("GPIO_0"); 

    lab2_init(gpio);

    while(1){
        k_sleep(K_SECONDS(1));
    }
}