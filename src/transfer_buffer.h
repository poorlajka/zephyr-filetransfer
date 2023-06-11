#ifndef FILE_BUFFER_H 
#define FILE_BUFFER_H

/*
 * A simple databuffer that can be iterated through (manually right now make it
 * automatic in the future it's easy but I didn't do it cuz I'm a stupid) :-). 
*/

struct TransferBuffer {
    char* buffer;
    ssize_t size;
    ssize_t position;
};

int transfer_buffer_load_file (struct TransferBuffer* transfer_buffer, char* path);

/*
 * Read a number of bytes from the current position. 
*/
int transfer_buffer_read (struct TransferBuffer* transfer_buffer, char* write_buffer, size_t read_len);

bool transfer_buffer_is_at_end (struct TransferBuffer* transfer_buffer);

#endif
