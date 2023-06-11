#ifndef FILE_H 
#define FILE_H

int lsdir(const char *path);

int file_sys_init(void);

void littlefs_unmount(void);

void file_sys_create_files (void);

void file_sys_rm_files (void);

#endif
