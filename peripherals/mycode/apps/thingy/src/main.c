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

#define PRIORITY 7
#define STACKSIZE 1024
#define MSG_SIZE 6

// Change between '1' and '2'
#define THINGY52 '2'


static const struct bt_data ad[] = {
	BT_DATA_BYTES(0x02, 0x01, 0x06, // Adv Flags, Standard
			0x03, 0x03, 0xAA, 0xFE, // Service Advertised
			0x17, 0x16, 0xaa, 0xfe, // Fixed length of 0x17, and UUID
			'h', 'e', 'y', 0x00, 0x00, // Name Space ID {1, 5}
			0x00, 0x00, 0x00, 0x00, 0x00, // Name Space ID {6, 10} (10 Bytes)
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00) // Instance ID (6 Bytes)
};

K_MSGQ_DEFINE(msgq, MSG_SIZE, 30, 4);

static char thingy_origin = THINGY52;

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
	thousand = tvoc.val1 / 1000;
	hundred = (tvoc.val1 / 100) % 10;
	ten = (tvoc.val1 / 10) % 10;
	one = tvoc.val1 % 10;

	itoa(thousand, thousandStr, 10);
	itoa(hundred, hundredStr, 10);
	itoa(ten, tenStr, 10);
	itoa(one, oneStr, 10);
	snprintf(output, 6, "%c%c%c%c%c", THINGY52, thousandStr[0], hundredStr[0], tenStr[0], oneStr[0]);

	//thingy_origin = THINGY52;
    k_msgq_put(&msgq, &output, K_NO_WAIT);
}

static char thingy_addr[20];
uint8_t found_address = 0;
/*
    Callback function which sends out the RSSI of iBeacon Nodes.
    Returns: None
*/
static void device_found(const bt_addr_le_t *addr, int8_t rssi, uint8_t type,
			 struct net_buf_simple *ad)
{
	
	char addr_str[BT_ADDR_LE_STR_LEN];
    char value[6];
    bt_addr_le_to_str(addr, addr_str, sizeof(addr_str));
    char* address_only = strtok(addr_str, " ");

    char string[20] = {ad->data[13],ad->data[14], ad->data[15],ad->data[16],ad->data[17],ad->data[18],ad->data[19]};

    uint8_t data_index = 25;
    value[1] = ad->data[data_index];
    value[2] = ad->data[data_index + 1];
    value[3] = ad->data[data_index + 2];
    value[4] = ad->data[data_index + 3];
    value[5] = '\0';
	// If the board 1 receives from 2, 
    if (strcmp(string, "THINGY1") == 0) {
		value[0] = '1';
    	k_msgq_put(&msgq, &value, K_NO_WAIT);
		
		k_sleep(K_MSEC(1000));
	} else if (strcmp(string, "THINGY2") == 0) {
		value[0] = '2';
    	k_msgq_put(&msgq, &value, K_NO_WAIT);
		
		k_sleep(K_MSEC(1000));
    }
	
}

/*
    Thread which waits until there is a message in the queue and sends it out via bluetooth
	in the EddyStone Protocol
    Returns: None
*/
void thread_update_bt(void) {
	char tx_buf[MSG_SIZE];
	while (k_msgq_get(&msgq, &tx_buf, K_FOREVER) == 0) {	
		struct bt_data update_ad[] = {
			BT_DATA_BYTES(BT_DATA_FLAGS, BT_LE_AD_NO_BREDR),
			BT_DATA_BYTES(BT_DATA_UUID16_ALL, 0xaa, 0xfe),
			BT_DATA_BYTES(BT_DATA_SVC_DATA16,
		    0xaa, 0xfe, /* Eddystone UUID */
		    0x00, 0x00, 
			'T', 'H', 'I', 'N', 'G', // Name Space ID {1, 5}
			'Y', THINGY52, '-', '-', '-', // Name Space ID {6, 10} (10 Bytes)
			'V', tx_buf[0], tx_buf[1], tx_buf[2], tx_buf[3], tx_buf[4], // Instance ID (6 Bytes)
		    0x00,0x00) // RSU
		};
		
		bt_le_adv_update_data(update_ad, ARRAY_SIZE(update_ad), NULL, 0);
		k_sleep(K_MSEC(10));
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
		
		k_sleep(K_MSEC(10));
	}
	k_sleep(K_FOREVER);
	return 0;
}
