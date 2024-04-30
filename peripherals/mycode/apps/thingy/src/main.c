#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>
#include <stdio.h>
#include <zephyr/sys/util.h>
#include <string.h>
#include <zephyr/sys/printk.h>
#include <stdlib.h>

#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/hci.h>

#ifndef IBEACON_RSSI
#define IBEACON_RSSI 0xc8
#endif

#define PRIORITY 7
#define STACKSIZE 1024
#define MSG_SIZE 5

#define THINGY52 "VOC1"

static struct bt_data ad[] = {
	BT_DATA_BYTES(BT_DATA_FLAGS, BT_LE_AD_NO_BREDR), 
	BT_DATA_BYTES(BT_DATA_MANUFACTURER_DATA,
		      0x4c, 0x00, /* Apple */
		      0x02, 0x15, /* iBeacon */
		      0x18, 0xee, 0x15, 0x16, /* UUID[15..12] */
		      0x01, 0x6b, /* UUID[11..10] */
		      0x4b, 0xec, /* UUID[9..8] */
		      0xad, 0x96, /* UUID[7..6] */
		      0xbc, 0xb9, 0x6d, 0x16, 0x6e, 0x97, /* UUID[5..0] */
		      0x00, 0x00, /* Major */
		      0x00, 0x00, /* Minor */
		      IBEACON_RSSI) /* Calibrated RSSI @ 1m */
};

K_MSGQ_DEFINE(msgq, MSG_SIZE, 30, 4);


/*
    Callback function which gets the TVOC sensor data and sends it into a message queue
    Returns: None
*/
static void process(const struct device *ccs_dev) {
	struct sensor_value tvoc;
	if (sensor_sample_fetch(ccs_dev) < 0 ) {
		printk("Sensor sample update error\n");
		return;
	}

	if (sensor_channel_get(ccs_dev, SENSOR_CHAN_VOC, &tvoc) < 0) {
		printk("Cannot read CCS811 TVOC channel\n");
		return;
	}
	char output[32], tenStr[10], thousandStr[10], hundredStr[10], oneStr[10];
	int thousand, hundred, ten, one;

	/* display TVOC */
	snprintf(output, 5, THINGY52);
    k_msgq_put(&msgq, &output, K_NO_WAIT);

	thousand = tvoc.val1 / 1000;
	hundred = (tvoc.val1 / 100) % 10;
	ten = (tvoc.val1 / 10) % 10;
	one = tvoc.val1 % 10;

	itoa(thousand, thousandStr, 10);
	itoa(hundred, hundredStr, 10);
	itoa(ten, tenStr, 10);
	itoa(one, oneStr, 10);
	snprintf(output, 5, "%c%c%c%c",thousandStr[0], hundredStr[0], tenStr[0], oneStr[0]);
    k_msgq_put(&msgq, &output, K_NO_WAIT);
}

/*
    Callback function which sends out the RSSI of iBeacon Nodes.
    Returns: None
*/
static void device_found(const bt_addr_le_t *addr, int8_t rssi, uint8_t type,
			 struct net_buf_simple *ad)
{
	char addr_str[BT_ADDR_LE_STR_LEN];
    bt_addr_le_to_str(addr, addr_str, sizeof(addr_str));
    char* address_only = strtok(addr_str, " ");


	/*
	printk("Device found: %s (RSSI %d), type %u, AD data len %u and Major Minor Data: %c%c%c%c\n",
	addr_str, rssi, type, ad->len, ad->data[25],ad->data[26],ad->data[27],ad->data[28]);
	*/
	
}



/*
    Thread which waits until there is a message in the queue and sends it out via bluetooth
	in the iBeacon Protocol
    Returns: None
*/
void thread_update_bt(void) {
	char tx_buf[MSG_SIZE];
	while (k_msgq_get(&msgq, &tx_buf, K_FOREVER) == 0) {
		struct bt_data update_ad[] = {
				BT_DATA_BYTES(BT_DATA_FLAGS, BT_LE_AD_NO_BREDR), BT_DATA_BYTES(BT_DATA_MANUFACTURER_DATA,
		      0x4c, 0x00, /* Apple */
		      0x02, 0x15, /* iBeacon */
		      0x18, 0xee, 0x15, 0x16, /* UUID[15..12] */
		      0x01, 0x6b, /* UUID[11..10] */
		      0x4b, 0xec, /* UUID[9..8] */
		      0xad, 0x96, /* UUID[7..6] */
		      0xbc, 0xb9, 0x6d, 0x16, 0x6e, 0x97, /* UUID[5..0] */
					tx_buf[0], tx_buf[1], /* Major */
					tx_buf[2], tx_buf[3], /* Minor */
		      IBEACON_RSSI) /* Calibrated RSSI @ 1m */
			};

		bt_le_adv_update_data(update_ad, ARRAY_SIZE(update_ad), NULL, 0);
		k_sleep(K_MSEC(500));
	}
	
}

K_THREAD_DEFINE(update_bl_id, STACKSIZE, thread_update_bt, NULL, NULL, NULL, PRIORITY, 0, 0);


/*
    Sets up the bluetooth advertising with the default iBeacon protocol
    Returns: None
*/
static void bt_ready(int err) {
	if (err) {
		printk("Bluetooth init failed (err %d)\n", err);
		return;
	}

	printk("Bluetooth initialized\n");

	/* Start advertising */
	err = bt_le_adv_start(BT_LE_ADV_NCONN, ad, ARRAY_SIZE(ad),
			      NULL, 0);
	if (err) {
		printk("Advertising failed to start (err %d)\n", err);
		return;
	}
	printk("iBeacon started\n");
}


/*
    The main loop.
    Returns: 1 -> Error
             0 -> No Error
*/
int main(void) {
	const struct device *const ccs_dev = DEVICE_DT_GET_ONE(ams_ccs811);

	int err;
	/* Initialize the Bluetooth Subsystem */
	err = bt_enable(bt_ready);
	if (err) {
		printk("Bluetooth init failed (err %d)\n", err);
	}

	if (!device_is_ready(ccs_dev)) {
		printf("Device %s is not ready\n", ccs_dev->name);
		return 0;
	}

	struct bt_le_scan_param scan_param = {
		.type       = BT_LE_SCAN_TYPE_PASSIVE,
        .options    = BT_LE_SCAN_OPT_NONE,
		.interval   = BT_GAP_SCAN_FAST_INTERVAL_MIN,
		.window     = BT_GAP_SCAN_FAST_WINDOW,
	};

	// Starts the scanning
    err = bt_le_scan_start(&scan_param, device_found);
	if (err) {
		printk("Start scanning failed (err %d)\n", err);
		return err;
	}

	while (1) {
		process(ccs_dev);
		k_sleep(K_MSEC(500));
	}
	k_sleep(K_FOREVER);
	return 0;
}
