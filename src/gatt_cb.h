#ifndef GATT_CALLBACKS_H
#define GATT_CALLBACKS_H

ssize_t gatt_cb_char_write (struct bt_conn *conn, struct bt_gatt_attr *attr,
			 const void *buf, uint16_t len, uint16_t offset,
			 uint8_t flags);

ssize_t gatt_cb_char_read (struct bt_conn *conn, const struct bt_gatt_attr *attr,
			void *buf, uint16_t len, uint16_t offset);

int gatt_cb_read_from_buffer(char* buffer, int len);

#endif
