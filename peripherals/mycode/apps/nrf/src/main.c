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

#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/i2c.h>
#include <stdint.h>

#include <zephyr/data/json.h>

LOG_MODULE_REGISTER(AHU);

#define SEN54 0x69

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
    int timestamp;
    char temperature[6];
    char humidity[6];
    char pressure[6];
    char tvoc[6];
};

static struct DataStruct data[3];





struct data {
    char temp[6];
    char humid[6];
    char tvoc[6];
    char pressure[6];
};
 
struct pc_data {
    char sensor_id[10];
    char timestamp[15];
    struct data data_info;
};
 
static const struct json_obj_descr data_descr[] = {
    JSON_OBJ_DESCR_PRIM(struct data, temp, JSON_TOK_STRING),
    JSON_OBJ_DESCR_PRIM(struct data, humid, JSON_TOK_STRING),
    JSON_OBJ_DESCR_PRIM(struct data, tvoc, JSON_TOK_STRING),
    JSON_OBJ_DESCR_PRIM(struct data, pressure, JSON_TOK_STRING),
};
 
static const struct json_obj_descr pc_data_descr[] = {
    JSON_OBJ_DESCR_PRIM(struct pc_data, sensor_id, JSON_TOK_STRING),
    JSON_OBJ_DESCR_PRIM(struct pc_data, timestamp, JSON_TOK_STRING),
    JSON_OBJ_DESCR_OBJECT(struct pc_data, data_info, data_descr),
};



/*
    Task 4.1.3 - Timer
*/

static long system_u_seconds = 0;


// Holds the system time in seconds
void increment_time(struct k_timer *timer) {
    system_u_seconds += 1;
}

K_TIMER_DEFINE(sys_timer, increment_time, NULL);

void timer_init(void) {
    // Task 4.1.3 timer start
    k_timer_start(&sys_timer, K_USEC(1), K_USEC(1));
}


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
    char string[20] = {ad->data[13],ad->data[14], ad->data[15],ad->data[16],ad->data[17],ad->data[18]};



    uint8_t data_index = 25;
    value[0] = ad->data[data_index];
    value[1] = ad->data[data_index + 1];
    value[2] = ad->data[data_index + 2];
    value[3] = ad->data[data_index + 3];
    value[4] = '\0';
    if (strcmp(string, "THINGY") == 0) {
        data[0].timestamp = system_u_seconds;
        //printk("Time: %i\n", system_u_seconds);
        switch (ad->data[23]) {
            case 'T':
                if (ad->data[24] == '1') {
                    snprintf(data[0].temperature, 6, "%2.1f", atof(value));
                } else if (ad->data[24] == '2') {
                    snprintf(data[1].temperature, 6, "%2.1f", atof(value));
                }
                break;
            case 'V':                
                if (ad->data[24] == '1') {
                    snprintf(data[0].tvoc, 6, "%i", atoi(value));
                } else if (ad->data[24] == '2') {
                    snprintf(data[1].tvoc, 6, "%i", atoi(value));
                }
                break;
            case 'H':
                if (ad->data[24] == '1') {
                    snprintf(data[0].humidity, 6, "%2.1f", atof(value));
                } else if (ad->data[24] == '2') {
                    snprintf(data[1].humidity, 6, "%2.1f", atof(value));
                }
                break;
            default:
            break;
        }
    }
}

/**
 * I2C Section
 * 
*/
const struct device *i2c = DEVICE_DT_GET(DT_NODELABEL(i2c0));


void i2c_get_info(void) {
    uint8_t buffer1[24];
    uint8_t ready[2] = {0x03,0xC4};
	struct i2c_msg msg[2];

	msg[0].buf = (uint8_t *)ready;
	msg[0].len = 2;
	msg[0].flags = I2C_MSG_WRITE;

	msg[1].buf = (uint8_t *)buffer1;
	msg[1].len = 24;
	msg[1].flags = I2C_MSG_RESTART | I2C_MSG_READ | I2C_MSG_STOP;

	i2c_transfer(i2c, msg, 2, SEN54);


/*

    printk("bytes:%i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i\n",
    buffer1[0],buffer1[1],buffer1[2],buffer1[3],buffer1[4],buffer1[5],buffer1[5],buffer1[6],
    buffer1[7],buffer1[8],buffer1[9],buffer1[9],buffer1[10],buffer1[11],buffer1[12],buffer1[13],
    buffer1[14],buffer1[15],buffer1[16],buffer1[17],buffer1[18],buffer1[19],buffer1[20],buffer1[21],
    buffer1[22],buffer1[23]);
    */
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
           //printk("Received: %s", rx_buf);
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
    timer_init();
    I2C_SPEED_SET(I2C_SPEED_STANDARD);
    //k_sleep(K_MSEC(500));
    int err, ret;
	err = bt_enable(NULL);
	if (err) {
		printk("Bluetooth init failed (err %d)\n", err);
		return 0;
	}

    if (!device_is_ready(i2c)) {
        LOG_ERR("Device not ready");
        return -1;
    }
    /*
    
    uint8_t buf[2] = { 0x00, 0x37};
    ret = i2c_write(i2c, buf, 2, SEN54);
    if (ret) {
        LOG_ERR("hmmm `i2c_write` failed: %d", ret);
        return ret;
    }
    k_sleep(K_MSEC(500));
    
    uint8_t ready[2] = {0x56, 0x07};
    ret = i2c_write(i2c, ready, 2, SEN54);
        if (ret) {
        LOG_ERR("hmmm `i2c_write` failed: %d", ret);
        return ret;
    }
    */
    struct bt_le_scan_param scan_param = {
		.type       = BT_LE_SCAN_TYPE_PASSIVE,
        .options    = BT_LE_SCAN_OPT_NONE,
		.interval   = BT_GAP_SCAN_FAST_INTERVAL_MIN,
		.window     = BT_GAP_SCAN_FAST_WINDOW,
	};
    bt_le_scan_start(&scan_param, device_receive);
    /*
    char string[150] = "{\"sensor_id\": \"VTOC\",\"timestamp\": \"YYYY-MM-DDTHH:NN:SS:Z\",\"data\": {\"temperature\":\"32.3\",\"humidity\": \"30\", \"tvoc\": \"322\", \"pressure\": \"302.5\"}}";
    struct pc_data data_settings;
    int expected_return_code = (1 << ARRAY_SIZE(pc_data_descr)) - 1;
    ret = json_obj_parse(string, sizeof(string), pc_data_descr, ARRAY_SIZE(pc_data_descr), &data_settings);
    */
    
    struct pc_data uart_info;
    strcpy(uart_info.sensor_id, "TVOC");
    strcpy(uart_info.timestamp, "randomtime");

    struct data information;
    strcpy(information.temp, "23.3");
    strcpy(information.humid, "53.3");
    strcpy(information.pressure, "302.4");
    strcpy(information.tvoc, "343");

    uart_info.data_info = information;
    
    /*
    char buffer[20];
    json_obj_encode(pc_data_descr, ARRAY_SIZE(pc_data_descr), &uart_info, NULL, NULL);
    printk("parsed: %s", buffer);
*/
    while(1) {
        //i2c_get_info();
        char json[300];
        sprintf(json, "{\"1\": [{\"Timestamp\": \"%i\",\"Temperature\": \"%s\",\"Humidity\": \"%s\",\"TVOC\": \"%i\"}],\"2\": [{\"Timestamp\": \"%i\",\"Temperature\": \"%2.1f\",\"Humidity\": \"%2.1f\",\"TVOC\": \"%i\"}],\"3\": [{\"Timestamp\": \"23\",\"Temperature\": \"25\",\"Humidity\": \"41\",\"TVOC\": \"232\"}]}\n"
        , data[0].timestamp, data[0].temperature,data[0].humidity, atoi(data[0].tvoc),data[1].timestamp,  atof(data[1].temperature), atof(data[1].humidity), atoi(data[1].tvoc));
        printk(json);
        send_str(uart0, json);
        k_sleep(K_MSEC(1000));

    }
	//printk("Bluetooth initialized\n");
    return 0;
}