#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/bluetooth/gatt.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/sys/printk.h>
#include <zephyr/kernel.h>

#define VAL_MAX_LEN 512

/*
 * Characteristic cb function called on write request
 * Used to select the path for the file to be read by the read_file characteristic 
 *
 * conn: The connection that is requesting to read
 * attr: The attribute that's being written
 * buf: Buffer with the data to write
 * len: Number of bytes in the buffer
 * offset: Offset to start writing from
 * flags: ?
 *
 * returns: number of bytes written, or BT_GATT_ERR() with ATT error code
 */
ssize_t gatt_cb_char_write (struct bt_conn *conn, struct bt_gatt_attr *attr,
			 const void *buf, uint16_t len, uint16_t offset,
			 uint8_t flags)
{
    uint8_t *char_value = attr->user_data;

    if (offset + len > VAL_MAX_LEN) 
        return BT_GATT_ERR(BT_ATT_ERR_INVALID_OFFSET);

    printk("Writing %d bytes to characteristic... \n", len);
    printk("\n");

    char file_path[VAL_MAX_LEN];
    memcpy(file_path, buf, len);
    file_path[len] = '\0'; 
    len++;

    printk("Value written: %s \n", file_path);

    memcpy(char_value + offset, file_path, len);

    return len;
}

/*
 * Characteristic cb function called on read request
 *
 * conn: The connection that is requesting to read
 * attr: The attribute that's being read
 * buf: Buffer to place the read result in
 * len: Length of data to read
 * offset: Offset to start reading from
 *
 * returns: number of bytes read, or BT_GATT_ERR() with ATT error code
 */
ssize_t gatt_cb_char_read (struct bt_conn *conn, const struct bt_gatt_attr *attr,
			void *buf, uint16_t len, uint16_t offset)
{
    const char *char_value = attr->user_data; 

    printk("Reading characteristic... \n");
    printk("\n");
    return bt_gatt_attr_read(conn, attr, buf, len, offset, char_value,
                             strlen(char_value));
}

