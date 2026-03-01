/*
 * This file is auto-generated. Modifications will be lost.
 *
 * See https://android.googlesource.com/platform/bionic/+/master/libc/kernel/
 * for more information.
 */
#ifndef _UAPI_LINUX_BLK_CRYPTO_H
#define _UAPI_LINUX_BLK_CRYPTO_H
#include <linux/ioctl.h>
#include <linux/types.h>
struct blk_crypto_import_key_arg {
  __u64 raw_key_ptr;
  __u64 raw_key_size;
  __u64 lt_key_ptr;
  __u64 lt_key_size;
  __u64 reserved[4];
};
struct blk_crypto_generate_key_arg {
  __u64 lt_key_ptr;
  __u64 lt_key_size;
  __u64 reserved[4];
};
struct blk_crypto_prepare_key_arg {
  __u64 lt_key_ptr;
  __u64 lt_key_size;
  __u64 eph_key_ptr;
  __u64 eph_key_size;
  __u64 reserved[4];
};
#define BLKCRYPTOIMPORTKEY _IOWR(0x12, 137, struct blk_crypto_import_key_arg)
#define BLKCRYPTOGENERATEKEY _IOWR(0x12, 138, struct blk_crypto_generate_key_arg)
#define BLKCRYPTOPREPAREKEY _IOWR(0x12, 139, struct blk_crypto_prepare_key_arg)
#endif
