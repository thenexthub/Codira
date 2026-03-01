/*
 * This file is auto-generated. Modifications will be lost.
 *
 * See https://android.googlesource.com/platform/bionic/+/master/libc/kernel/
 * for more information.
 */
#ifndef _UAPI_FWCTL_H
#define _UAPI_FWCTL_H
#include <linux/types.h>
#include <linux/ioctl.h>
#define FWCTL_TYPE 0x9A
enum {
  FWCTL_CMD_BASE = 0,
  FWCTL_CMD_INFO = 0,
  FWCTL_CMD_RPC = 1,
};
enum fwctl_device_type {
  FWCTL_DEVICE_TYPE_ERROR = 0,
  FWCTL_DEVICE_TYPE_MLX5 = 1,
  FWCTL_DEVICE_TYPE_CXL = 2,
  FWCTL_DEVICE_TYPE_PDS = 4,
};
struct fwctl_info {
  __u32 size;
  __u32 flags;
  __u32 out_device_type;
  __u32 device_data_len;
  __aligned_u64 out_device_data;
};
#define FWCTL_INFO _IO(FWCTL_TYPE, FWCTL_CMD_INFO)
enum fwctl_rpc_scope {
  FWCTL_RPC_CONFIGURATION = 0,
  FWCTL_RPC_DEBUG_READ_ONLY = 1,
  FWCTL_RPC_DEBUG_WRITE = 2,
  FWCTL_RPC_DEBUG_WRITE_FULL = 3,
};
struct fwctl_rpc {
  __u32 size;
  __u32 scope;
  __u32 in_len;
  __u32 out_len;
  __aligned_u64 in;
  __aligned_u64 out;
};
#define FWCTL_RPC _IO(FWCTL_TYPE, FWCTL_CMD_RPC)
#endif
