#include <zephyr/kernel.h>
#include <zephyr/shell/shell.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <zephyr/drivers/gpio.h>

#include <zephyr/device.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/logging/log.h>

#include <zephyr/sys/printk.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/bluetooth/conn.h>

LOG_MODULE_REGISTER(AHU);

// Threads Parameters
#define STACKSIZE 1024
#define PRIORITY 7

// UART Parameters
#define MSG_SIZE 32

struct uart_config uart_cfg = 
{
    .baudrate = 115200,
    .parity = UART_CFG_PARITY_NONE,
    .stop_bits = UART_CFG_STOP_BITS_1,
	.flow_ctrl = UART_CFG_FLOW_CTRL_NONE,
	.data_bits = UART_CFG_DATA_BITS_8,
};

const struct device *uart0 = DEVICE_DT_GET(DT_NODELABEL(uart0));
//const struct device *uart1 = DEVICE_DT_GET(DT_NODELABEL(uart1));

/* queue to store up to 10 messages (aligned to 4-byte boundary) */
K_MSGQ_DEFINE(uart_msgq, MSG_SIZE, 10, 4);
K_MSGQ_DEFINE(rec_msgq, MSG_SIZE, 10, 4);



struct DataStruct {
    char voc1[6];
    char voc2[6];
};

static struct DataStruct data;
static char connection[20];

static char thingy_addr1[20], thingy_addr2[20];
static int found_address1 = 0, found_address2 = 0;


static uint8_t data_read, thingy;

/*
    Callback function which imports the data from the bluetooth device iBeacon signal.
    Returns: None
*/
static void device_receive(const bt_addr_le_t *addr, int8_t rssi, uint8_t type,
			 struct net_buf_simple *ad) {
	char addr_str[BT_ADDR_LE_STR_LEN];
    char value[5];
    bt_addr_le_to_str(addr, addr_str, sizeof(addr_str));
    char* address_only = strtok(addr_str, " ");
    char string[20] = {ad->data[13],ad->data[14], ad->data[15],ad->data[16],ad->data[17],ad->data[18],ad->data[19]};



    uint8_t data_index = 25;
    value[0] = ad->data[data_index];
    value[1] = ad->data[data_index + 1];
    value[2] = ad->data[data_index + 2];
    value[3] = ad->data[data_index + 3];
    value[4] = '\0';
    if (strcmp(string, "THINGY1") == 0) {
        if (ad->data[24] == '1') {
            printk("VOC1 Detected, TVOC: %s\n", value);
            snprintf(data.voc1, 6, "%i", atoi(value));
        } else if (ad->data[24] == '1') {
            printk("VOC2 Detected, TVOC: %s\n", value);
            snprintf(data.voc2, 6, "%i", atoi(value));
        }
    } else if (strcmp(string, "THINGY2") == 0) {
        if (ad->data[24] == '1') {
            printk("VOC1 Detected, TVOC: %s\n", value);
            snprintf(data.voc1, 6, "%i", atoi(value));
        } else if (ad->data[24] == '1') {
            printk("VOC2 Detected, TVOC: %s\n", value);
            snprintf(data.voc2, 6, "%i", atoi(value));
        }
    }
}




/*

    Task 4.1.6 - Zephyr OS Library

*/
/*
    Sends a string to UART
    Returns: None
*/
void send_str(const struct device *uart, char *str) {
	int msg_len = strlen(str);
	for (int i = 0; i < msg_len; i++) {
		uart_poll_out(uart, str[i]);
	}
}


/*
    Thread which constantly waits for message in queue and sends it.
    Returns: None
*/
void thr_uart_send(void) {
	char tx_buf[MSG_SIZE];

	if (!device_is_ready(uart0)) {
		LOG_ERR("UART device not found!");
		return;
	}

	while (k_msgq_get(&uart_msgq, &tx_buf, K_FOREVER) == 0) {
        send_str(uart0, tx_buf);
	}
}

K_THREAD_DEFINE(uart_id, STACKSIZE, thr_uart_send, NULL, NULL, NULL, PRIORITY, 0, 0);


/*

    Task 4.1.7 - Communication Interface

*/
/*
    Receives the string from the uart RX buffer
    Returns: None
*/
void recv_str(const struct device *uart, char *str) {
	char *head = str;
	char c;
	while (!uart_poll_in(uart, &c)) {
		*head++ = c;
	}
	*head = '\0';
}


/*
    Thread which constantly waits until a uart RX buffer receives something and prints it out
    Returns: None
*/
void thr_receive(void) {
	char rx_buf[MSG_SIZE];

	if (!device_is_ready(uart0)) {
		LOG_ERR("UART device not found!");
		return;
	}

	while (device_is_ready(uart0)) {
        k_sleep(K_MSEC(500));
        recv_str(uart0, rx_buf);
        if (strcmp(rx_buf, "") != 0) {
            k_msgq_put(&rec_msgq, &rx_buf, K_NO_WAIT);
            printk("Received: %s", rx_buf);
        }
	}
}

K_THREAD_DEFINE(rec_id, STACKSIZE, thr_receive, NULL, NULL, NULL, PRIORITY, 0, 0);

/*
    The main loop.
    Returns: 1 -> Error
             0 -> No Error
*/
int main (void) {

    int err;
	err = bt_enable(NULL);
	if (err) {
		printk("Bluetooth init failed (err %d)\n", err);
		return 0;
	}

    // Comment underneath to display ibeacon add functions
    struct bt_le_scan_param scan_param = {
		.type       = BT_LE_SCAN_TYPE_PASSIVE,
        .options    = BT_LE_SCAN_OPT_NONE,
		.interval   = BT_GAP_SCAN_FAST_INTERVAL_MIN,
		.window     = BT_GAP_SCAN_FAST_WINDOW,
	};
    
    bt_le_scan_start(&scan_param, device_receive);

	printk("Bluetooth initialized\n");
    return 0;
}