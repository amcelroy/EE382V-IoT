#ifndef __LAB2_H__
#define __LAB2_H__

#include <ctype.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/conn.h>
#include <bluetooth/uuid.h>
#include <bluetooth/gatt.h>
#include <bluetooth/services/bas.h>
#include <bluetooth/services/hrs.h>
#include <drivers/led.h>
#include <drivers/gpio.h>
#include <power/power.h>

int lab2_init(struct device *gpio);

#endif