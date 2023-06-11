#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/bluetooth/gatt.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/sys/printk.h>

#define BT_UUID_CUSTOM_SERVICE_VAL \
	BT_UUID_128_ENCODE(0x00000001,0x1000,0x2000,0x3000,0x111122223333)

/* Bluetooth advertising data */
static const struct bt_data ad[] = {
    BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
    BT_DATA_BYTES(BT_DATA_UUID16_ALL),
    BT_DATA_BYTES(BT_DATA_UUID128_ALL, BT_UUID_CUSTOM_SERVICE_VAL),
};

/* Bluetooth connection display callback */
static void auth_passkey_display (struct bt_conn *conn, unsigned int passkey) {
    char addr[BT_ADDR_LE_STR_LEN];

    bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

    printk("Passkey for %s: %06u\n", addr, passkey);
    printk("\n");
}

/* Bluetooth connection authentication cancel callback */
static void auth_cancel (struct bt_conn *conn) {
    char addr[BT_ADDR_LE_STR_LEN];

    bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

    printk("Pairing cancelled: %s\n", addr);
    printk("\n");
}

/*
static struct bt_conn_auth_cb auth_cb_display = {
    .passkey_display = auth_passkey_display,
    .passkey_entry = NULL,
    .cancel = auth_cancel,
};
*/


/* Bluetooth connected callback */
static void connected (struct bt_conn *conn, uint8_t err) {
    if (err) {
            printk("Connection failed (err %u)\n", err);
            return;
    }

    printk("Connected\n");
    printk("\n");
}

/* Bluetooth disconected callback */
static void disconnected (struct bt_conn *conn, uint8_t reason) {
    printk("Disconnected (reason %u)\n", reason);
    printk("\n");
}

BT_CONN_CB_DEFINE (conn_callbacks) = {
    .connected = connected,
    .disconnected = disconnected,
};


/* Gatt connection callback */
void mtu_updated (struct bt_conn *conn, uint16_t tx, uint16_t rx) {
    printk("Updated MTU: TX: %d RX: %d bytes\n", tx, rx);
    printk("\n");
}

static struct bt_gatt_cb gatt_callbacks = {
    .att_mtu_updated = mtu_updated
};

int ble_setup_init (void) {
    int err = 1;

    printk("Starting Bluetooth setup \n");
    printk("\n");

    
    err = bt_enable(NULL);
    if (err) {
        printk("Bluetooth init failed (err %d)\n", err);
        printk("\n");
        return err;
    }
    printk("Bluetooth initialized\n");
    printk("\n");
    bt_set_name("Buss sign");

    ble_setup_register_callbacks();

    return err;
}

int ble_setup_start_advertising (void) {
    int err;

    if (IS_ENABLED(CONFIG_SETTINGS)) {
        settings_load();
    }
    err = bt_le_adv_start(BT_LE_ADV_CONN_NAME, ad, ARRAY_SIZE(ad), NULL, 0);
    if (err) {
        printk("Advertising failed to start (err %d)\n", err);
        printk("\n");
        return err;
    }
    printk("Advertising successfully started\n");
    printk("\n");

    //settings_load();
    return err;
}

void ble_setup_register_callbacks (void) {
    bt_gatt_cb_register(&gatt_callbacks);
    //bt_conn_auth_cb_register(&auth_cb_display);
}

