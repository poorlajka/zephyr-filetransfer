#ifndef GATT_SERVICE_H
#define GATT_SERVICE_H

/*
 * Condition for a client requesting datatransfer.
 */
bool gatt_serv_is_transfer_ready (void);

/*
 * Loads data from filepath given by file_select_attr to the transfer_buffer.
 */
int gatt_serv_load_transfer_data (void);

/*
 * Send 20 bytes of data from transfer_buffer at current transfer_buffer.position 
 * to client and increment transfer_buffer.position.  
 */
int gatt_serv_send_packet (void);

#endif
