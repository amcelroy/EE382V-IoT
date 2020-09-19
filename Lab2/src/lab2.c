#include "lab2.h"

uint8_t tx_buffer[64];

uint8_t loopback_buffer[64];

struct bt_uuid_128 serviceUUID = BT_UUID_INIT_128(
 	0xDE, 0x40, 0x00, 0x02, 0xB5, 0xA3, 0xF3, 0x93,
 	0xE0, 0xA9, 0x02, 0x0EF, 0xEF, 0xBE, 0xAD, 0xDE);

struct bt_uuid_128 ledOnUUID = BT_UUID_INIT_128(
	0xDE, 0x40, 0x00, 0x02, 0xB5, 0xA3, 0xF3, 0x93,
 	0xE0, 0xA9, 0x01, 0x0EF, 0xEF, 0xBE, 0xAD, 0xDE);

struct bt_uuid_128 txUUID = BT_UUID_INIT_128(
	0xDE, 0x40, 0x00, 0x02, 0xB5, 0xA3, 0xF3, 0x93,
 	0xE0, 0xA9, 0x01, 0x01, 0xEF, 0xBE, 0xAD, 0xDE);

struct bt_uuid_128 loopbackUUID = BT_UUID_INIT_128(
	0xDE, 0x40, 0x00, 0x02, 0xB5, 0xA3, 0xF3, 0x93,
 	0xE0, 0xA9, 0x01, 0x02, 0xEF, 0xBE, 0xAD, 0xDE);

static const struct bt_data service_ad[] = {
	BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
	BT_DATA_BYTES(BT_DATA_UUID128_ALL,
        0xDE, 0x40, 0x00, 0x02, 0xB5, 0xA3, 0xF3, 0x93,
        0xE0, 0xA9, 0xAD, 0x0EF, 0xEF, 0xBE, 0xAD, 0xDE)
};

static ssize_t callback_read(struct bt_conn *conn, const struct bt_gatt_attr *attr, void *buf, uint16_t len, uint16_t offset);

static ssize_t callback_led_write(struct bt_conn *conn, const struct bt_gatt_attr *attr, void *buf, uint16_t len, uint16_t offset, uint8_t flags);

static ssize_t callback_tx_write(struct bt_conn *conn, const struct bt_gatt_attr *attr, void *buf, uint16_t len, uint16_t offset, uint8_t flags);

static ssize_t callback_loopback(struct bt_conn *conn, const struct bt_gatt_attr *attr, void *buf, uint16_t len, uint16_t offset);

bool led_status = false;

struct device *led_device = 0;

BT_GATT_SERVICE_DEFINE(lab2_blueooth_svc,
    BT_GATT_PRIMARY_SERVICE(&serviceUUID),
    BT_GATT_CHARACTERISTIC(&ledOnUUID.uuid, 
        BT_GATT_CHRC_READ | BT_GATT_CHRC_NOTIFY | BT_GATT_CHRC_WRITE,
        BT_GATT_PERM_READ | BT_GATT_PERM_WRITE,
        callback_read, 
        callback_led_write, 
        &led_status
    ),
    BT_GATT_CHARACTERISTIC(&txUUID.uuid,
        BT_GATT_CHRC_WRITE,
        BT_GATT_PERM_WRITE,
        NULL,
        callback_tx_write,
        tx_buffer
    ),
    BT_GATT_CHARACTERISTIC(&loopbackUUID.uuid,
        BT_GATT_CHRC_READ | BT_GATT_CHRC_NOTIFY,
        BT_GATT_PERM_READ,
        callback_loopback,
        NULL,
        loopback_buffer
    ),
);

static ssize_t callback_read(struct bt_conn *conn, const struct bt_gatt_attr *attr, void *buf, uint16_t len, uint16_t offset){
    uint8_t *value = attr->user_data;
    
    return bt_gatt_attr_read(
        conn,
        attr,
        buf,
        len,
        offset,
        value,
        sizeof(bool)
    );
}

static ssize_t callback_led_write(struct bt_conn *conn, const struct bt_gatt_attr *attr, void *buf, uint16_t len, uint16_t offset, uint8_t flags){
    bool *value = (bool*)buf;

    led_status = *value;

    if(led_status){
        gpio_pin_set(led_device, 16, 1);
    }else{
        gpio_pin_set(led_device, 16, 0);
    }

    return len;
}

static ssize_t callback_tx_write(struct bt_conn *conn, const struct bt_gatt_attr *attr, void *buf, uint16_t len, uint16_t offset, uint8_t flags){
    bool *value = (bool*)buf;

    memcpy(loopback_buffer, buf, len);

    bt_gatt_notify(conn, attr, loopback_buffer, len);

    return len;
}

static ssize_t callback_loopback(struct bt_conn *conn, const struct bt_gatt_attr *attr, void *buf, uint16_t len, uint16_t offset){
    uint8_t *value = attr->user_data;
    
    return bt_gatt_attr_read(
        conn,
        attr,
        buf,
        len,
        offset,
        value,
        64
    );
}

int lab2_init(struct device *gpio){
    led_device = gpio;

    gpio_pin_configure(led_device, 14, GPIO_OUTPUT);
    gpio_pin_configure(led_device, 15, GPIO_OUTPUT);
    gpio_pin_configure(led_device, 16, GPIO_OUTPUT);

    gpio_pin_set(led_device, 16, 1);
    k_sleep(K_SECONDS(1));
    gpio_pin_set(led_device, 16, 0);

    int err = 0;
	err = bt_enable(NULL);
	if (err) {
		printk("Bluetooth init failed (err %d)\n", err);
		return err;
	}
	printk("Bluetooth initialized");

	err = bt_le_adv_start(BT_LE_ADV_CONN_NAME, service_ad, ARRAY_SIZE(service_ad), NULL, 0);
	if (err) {
		printk("Advertising failed to start (err %d)\n", err);
		return err;
	}
	printk("Bluetooth advertising");

    return err;
}