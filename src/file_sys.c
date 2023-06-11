/*
 * Copyright (c) 2022 Lukasz Majewski, DENX Software Engineering GmbH
 * Copyright (c) 2019 Peter Bigot Consulting, LLC
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/* Sample which uses the filesystem API with littlefs */

#include <stdio.h>

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/fs/fs.h>
#include <zephyr/fs/littlefs.h>
#include <zephyr/logging/log.h>
#include <zephyr/storage/flash_map.h>
#include <string.h>

#include "example_file_data.h"

LOG_MODULE_REGISTER(fs_main);

/* Matches LFS_NAME_MAX */
#define MAX_PATH_LEN 255
#define TEST_FILE_SIZE 547

static uint8_t file_test_pattern[TEST_FILE_SIZE];
int lsdir(const char *path)
{
	int res;
	struct fs_dir_t dirp;
	static struct fs_dirent entry;

	fs_dir_t_init(&dirp);

	/* Verify fs_opendir() */
	res = fs_opendir(&dirp, path);
	if (res) {
		LOG_ERR("Error opening dir %s [%d]\n", path, res);
		return res;
	}

	LOG_PRINTK("\nListing dir %s ...\n", path);
	for (;;) {
		/* Verify fs_readdir() */
		res = fs_readdir(&dirp, &entry);

		/* entry.name[0] == 0 means end-of-dir */
		if (res || entry.name[0] == 0) {
			if (res < 0) {
				LOG_ERR("Error reading dir [%d]\n", res);
			}
			break;
		}

		if (entry.type == FS_DIR_ENTRY_DIR) {
			LOG_PRINTK("[DIR ] %s\n", entry.name);
		} else {
			LOG_PRINTK("[FILE] %s (size = %zu)\n",
				   entry.name, entry.size);
		}
	}

	/* Verify fs_closedir() */
	fs_closedir(&dirp);

	return res;
}

#ifdef CONFIG_APP_LITTLEFS_STORAGE_FLASH
static int littlefs_flash_erase(unsigned int id)
{
	const struct flash_area *pfa;
	int rc;

	rc = flash_area_open(id, &pfa);
	if (rc < 0) {
		LOG_ERR("FAIL: unable to find flash area %u: %d\n",
			id, rc);
		return rc;
	}

	LOG_PRINTK("Area %u at 0x%x on %s for %u bytes\n",
		   id, (unsigned int)pfa->fa_off, pfa->fa_dev->name,
		   (unsigned int)pfa->fa_size);

	/* Optional wipe flash contents */
	if (IS_ENABLED(CONFIG_APP_WIPE_STORAGE)) {
		rc = flash_area_erase(pfa, 0, pfa->fa_size);
		LOG_ERR("Erasing flash area ... %d", rc);
	}

	flash_area_close(pfa);
	return rc;
}
#define PARTITION_NODE DT_NODELABEL(lfs1)

#if DT_NODE_EXISTS(PARTITION_NODE)
FS_FSTAB_DECLARE_ENTRY(PARTITION_NODE);
#else /* PARTITION_NODE */
FS_LITTLEFS_DECLARE_DEFAULT_CONFIG(storage);
static struct fs_mount_t lfs_storage_mnt = {
	.type = FS_LITTLEFS,
	.fs_data = &storage,
	.storage_dev = (void *)FIXED_PARTITION_ID(storage_partition),
	.mnt_point = "/lfs",
};
#endif /* PARTITION_NODE */

	struct fs_mount_t *mp =
#if DT_NODE_EXISTS(PARTITION_NODE)
		&FS_FSTAB_ENTRY(PARTITION_NODE)
#else
		&lfs_storage_mnt
#endif
		;

static int littlefs_mount(struct fs_mount_t *mp)
{
	int rc;

	rc = littlefs_flash_erase((uintptr_t)mp->storage_dev);
	if (rc < 0) {
		return rc;
	}

	/* Do not mount if auto-mount has been enabled */
#if !DT_NODE_EXISTS(PARTITION_NODE) ||						\
	!(FSTAB_ENTRY_DT_MOUNT_FLAGS(PARTITION_NODE) & FS_MOUNT_FLAG_AUTOMOUNT)
	rc = fs_mount(mp);
	if (rc < 0) {
		LOG_PRINTK("FAIL: mount id %" PRIuPTR " at %s: %d\n",
		       (uintptr_t)mp->storage_dev, mp->mnt_point, rc);
		return rc;
	}
	LOG_PRINTK("%s mount: %d\n", mp->mnt_point, rc);
#else
	LOG_PRINTK("%s automounted\n", mp->mnt_point);
#endif

	return 0;
}
#endif /* CONFIG_APP_LITTLEFS_STORAGE_FLASH */

#ifdef CONFIG_APP_LITTLEFS_STORAGE_BLK_SDMMC
struct fs_littlefs lfsfs;
static struct fs_mount_t __mp = {
	.type = FS_LITTLEFS,
	.fs_data = &lfsfs,
	.flags = FS_MOUNT_FLAG_USE_DISK_ACCESS,
};
struct fs_mount_t *mp = &__mp;

static int littlefs_mount(struct fs_mount_t *mp)
{
	static const char *disk_mount_pt = "/"CONFIG_SDMMC_VOLUME_NAME":";
	static const char *disk_pdrv = CONFIG_SDMMC_VOLUME_NAME;

	mp->storage_dev = (void *)disk_pdrv;
	mp->mnt_point = disk_mount_pt;

	return fs_mount(mp);
}
#endif /* CONFIG_APP_LITTLEFS_STORAGE_BLK_SDMMC */


void file_sys_create_files (void) {
        if (fs_mkdir("/lfs/sys_logs") == -17) {
            return;
        }
        int ret;

        struct fs_file_t file;

        fs_file_t_init(&file);
        ret = fs_open(&file, "/lfs/sys_logs/log1.json", FS_O_RDWR | FS_O_CREATE);
        ret = fs_write(&file, json_data, json_data_len);
        ret = fs_close(&file);

        fs_file_t_init(&file);
        ret = fs_open(&file, "/lfs/sys_logs/log2.txt", FS_O_RDWR | FS_O_CREATE);
        ret = fs_write(&file, error_data1, error_data1_len);
        ret = fs_close(&file);

        fs_file_t_init(&file);
        ret = fs_open(&file, "/lfs/sys_logs/log3.txt", FS_O_RDWR | FS_O_CREATE);
        ret = fs_write(&file, error_data2, error_data2_len);
        ret = fs_close(&file);
}

void file_sys_rm_files (void) {
    fs_unlink("/lfs/sys_logs/log1.json");
    fs_unlink("/lfs/sys_logs/log2.txt");
    fs_unlink("/lfs/sys_logs/log3.txt");
    fs_unlink("/lfs/sys_logs");
}

int file_sys_init(void)
{
	struct fs_statvfs sbuf;
	int rc;

	LOG_PRINTK("Sample program to r/w files on littlefs\n");

	rc = littlefs_mount(mp);

	if (rc < 0) {
		return rc;
	}

	rc = fs_statvfs(mp->mnt_point, &sbuf);
	if (rc < 0) {
		LOG_PRINTK("FAIL: statvfs: %d\n", rc);
		return rc;
	}

	LOG_PRINTK("%s: bsize = %lu ; frsize = %lu ;"
		   " blocks = %lu ; bfree = %lu\n",
		   mp->mnt_point,
		   sbuf.f_bsize, sbuf.f_frsize,
		   sbuf.f_blocks, sbuf.f_bfree);

	rc = lsdir(mp->mnt_point);
	if (rc < 0) {
		LOG_PRINTK("FAIL: lsdir %s: %d\n", mp->mnt_point, rc);
		return rc;
	}

        return rc;

}

void littlefs_unmount(void) {
    int rc = fs_unmount(mp);
}
