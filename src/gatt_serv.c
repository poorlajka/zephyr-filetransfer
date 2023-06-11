#include <zephyr/bluetooth/gatt.h>
#include "gatt_cb.h"
#include "transfer_buffer.h"

#define MAX_ATTR_VAL_SIZE 512

/*
 * Service attribute identifier
*/
#define BT_UUID_CUSTOM_SERVICE_VAL \
	BT_UUID_128_ENCODE(0x00000001,0x1000,0x2000,0x3000,0x111122223333)

static struct bt_uuid_128 serv_attr_uuid = BT_UUID_INIT_128(
	BT_UUID_CUSTOM_SERVICE_VAL);

/*
 * Charactaristic attribute identifiers
*/
static struct bt_uuid_128 file_select_attr_uuid = BT_UUID_INIT_128(
	BT_UUID_128_ENCODE(0x12345678, 0x1234, 0x5678, 0x1234, 0x56789abcdef1));

static struct bt_uuid_128 file_read_attr_uuid = BT_UUID_INIT_128(
	BT_UUID_128_ENCODE(0x12345678, 0x1234, 0x5678, 0x1234, 0x56789abcdef2));

/*
 * Notification subscription callback
*/
static uint8_t is_notifying;
static void file_read_ccc_changed (const struct bt_gatt_attr *attr, uint16_t value) {
    is_notifying = value;
}

static uint8_t char_test_val[21] = {'R', 'A', 'N', 'D', 'O', 'M', ':', ')'};

/* 
 * Service definition 
*/
BT_GATT_SERVICE_DEFINE(data_transfer_service,

    BT_GATT_PRIMARY_SERVICE(&serv_attr_uuid),

    /* File selecting characteristic */
    BT_GATT_CHARACTERISTIC(&file_select_attr_uuid.uuid,
                           BT_GATT_CHRC_WRITE | BT_GATT_CHRC_READ, BT_GATT_PERM_WRITE | BT_GATT_PERM_READ,
                            gatt_cb_char_read, gatt_cb_char_write, char_test_val),

    /* File reading characteristic */
    BT_GATT_CHARACTERISTIC(&file_read_attr_uuid.uuid,
                           BT_GATT_CHRC_NOTIFY, BT_GATT_PERM_NONE, 
                           NULL, NULL, NULL),
    BT_GATT_CCC(file_read_ccc_changed, BT_GATT_PERM_READ | BT_GATT_PERM_WRITE),
);

struct TransferBuffer transfer_buffer = {
    .buffer = NULL,
    .size = 0,
    .position = 0
};

/* 
 * Condition for starting to transfer data
*/
bool gatt_serv_is_transfer_ready (void) {
    bool is_file_path_set = data_transfer_service.attrs[2].user_data != NULL;
    return is_notifying && is_file_path_set;
}

int gatt_serv_load_transfer_data (void) {
    int rc = transfer_buffer_load_file(&transfer_buffer, data_transfer_service.attrs[2].user_data);
    //bt_gatt_notify(NULL, &data_transfer_service.attrs[4], &transfer_buffer->size, sizeof(transfer_buffer->size));
    return rc;
}

int gatt_serv_send_packet (void) {
    char data_packet[20];
    
    int bytes_read = transfer_buffer_read(&transfer_buffer, &data_packet, 20); 
    int rc = 0;


    if (bytes_read == -1) {
        memset(&data_packet, '0', 20);
        is_notifying = false;
        bytes_read = 20;

        transfer_buffer.position = 0;
        rc = -1;
        //return rc;
    }

    bt_gatt_notify(NULL, &data_transfer_service.attrs[4], &data_packet, bytes_read);
    return rc;
}

