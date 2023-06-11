#include <zephyr/sys/printk.h>
#include <zephyr/kernel.h>
#include <zephyr/fs/fs.h>

#include "ble_setup.h"
#include "gatt_serv.h"
#include "file_sys.h"

/*
 * Hello there, the filetransfer was implemented using a null packet to signify
 * transmittion end, this is stupid since
 */

enum ServerState {
    Idle,
    TransferringData,
};

/*
 * Declaration and registration of GATT-service can be found in gatt_serv.h. 
 * void
 */
void main (void) {

    /* 
     * Setup bluetooth and filesys.
     */
    if (ble_setup_init()) {
        return;
    }
    if (ble_setup_start_advertising()) {
        return;
    }
    if (file_sys_init() < 0) {
        return;
    }

    /*
     * Uncomment these to create and remove example files for 
     * testing the filesystem. 
     */
    //file_sys_create_files();
    //file_sys_rm_files();
    lsdir("/lfs/sys_logs");

    /*
     * Server will simply wait for a client to request filedata,
     * upon which it will read data from disk and perform transfer to client.
     */
    enum ServerState server_state = Idle;
    while (true) {
        switch (server_state) {
            case Idle:
                if (gatt_serv_is_transfer_ready()) {

                    gatt_serv_load_transfer_data();

                    server_state = TransferringData;
                }
                k_sleep(K_MSEC(100));
                break;

            case TransferringData:
                if (gatt_serv_send_packet()) {
                    server_state = Idle;
                }
                break;
        }
    }
}


