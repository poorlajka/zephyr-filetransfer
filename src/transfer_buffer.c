#include <string.h>
#include <sys/types.h>
#include <zephyr/sys/printk.h>
#include <zephyr/fs/fs.h>
#include <zephyr/kernel.h>
#include "transfer_buffer.h"

void transfer_buffer_clear(struct TransferBuffer* transfer_buffer) {
    transfer_buffer->size = 0;
    transfer_buffer->position = 0;
    transfer_buffer->buffer = NULL;
}

int transfer_buffer_load_file (struct TransferBuffer* transfer_buffer, char* file_path) {

    if (transfer_buffer->size != 0) {
        k_free(transfer_buffer->buffer);
    }

    int rc;
    struct fs_dirent entry;
    rc = fs_stat(file_path, &entry);
    if (rc) {
        transfer_buffer_clear(transfer_buffer);
        return rc;
    }
    size_t file_size = entry.size;


    struct fs_file_t file;
    fs_file_t_init(&file);


    rc = fs_open(&file, file_path, FS_O_READ);
    if (rc) { 
        transfer_buffer_clear(transfer_buffer);
        return rc;
    }

    transfer_buffer->buffer = k_malloc(file_size);

    rc = fs_read(&file, transfer_buffer->buffer, file_size);
    if (rc < 0) {
        transfer_buffer_clear(transfer_buffer);
        return rc;
    }

    transfer_buffer->size = file_size;

    rc = fs_close(&file);
    if (rc) {
        transfer_buffer_clear(transfer_buffer);
        return rc;
    }

    return 0;
}

bool transfer_buffer_is_at_end (struct TransferBuffer* transfer_buffer) {
    return transfer_buffer->size == transfer_buffer->position;
}

int transfer_buffer_read (struct TransferBuffer* transfer_buffer, char* write_buffer, size_t read_len) {

    if (transfer_buffer_is_at_end(transfer_buffer))
        return -1;

    int buffer_len = transfer_buffer->size;
    int offset = transfer_buffer->position;

    if (buffer_len < offset + read_len) 
        read_len = buffer_len - offset;

    memcpy(write_buffer, &transfer_buffer->buffer[offset], read_len);

    transfer_buffer->position += read_len;

    return read_len;
}


