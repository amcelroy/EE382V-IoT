#include "lab2.h"
#include "power_management.h"

void main(int argc, char **argv){

    struct device *gpio = device_get_binding("GPIO_0"); 

    power_management_init();

    lab2_init(gpio);

    //sys_set_power_state(SYS_POWER_STATE_SLEEP_1);

    while(1){
         k_sleep(K_SECONDS(1));
    }
}