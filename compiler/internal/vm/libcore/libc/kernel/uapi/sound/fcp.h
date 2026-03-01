/*
 * This file is auto-generated. Modifications will be lost.
 *
 * See https://android.googlesource.com/platform/bionic/+/master/libc/kernel/
 * for more information.
 */
#ifndef __UAPI_SOUND_FCP_H
#define __UAPI_SOUND_FCP_H
#include <linux/types.h>
#include <linux/ioctl.h>
#define FCP_HWDEP_MAJOR 2
#define FCP_HWDEP_MINOR 0
#define FCP_HWDEP_SUBMINOR 0
#define FCP_HWDEP_VERSION ((FCP_HWDEP_MAJOR << 16) | (FCP_HWDEP_MINOR << 8) | FCP_HWDEP_SUBMINOR)
#define FCP_HWDEP_VERSION_MAJOR(v) (((v) >> 16) & 0xFF)
#define FCP_HWDEP_VERSION_MINOR(v) (((v) >> 8) & 0xFF)
#define FCP_HWDEP_VERSION_SUBMINOR(v) ((v) & 0xFF)
#define FCP_IOCTL_PVERSION _IOR('S', 0x60, int)
struct fcp_init {
  __u16 step0_resp_size;
  __u16 step2_resp_size;
  __u32 init1_opcode;
  __u32 init2_opcode;
  __u8 resp[];
} __attribute__((packed));
#define FCP_IOCTL_INIT _IOWR('S', 0x64, struct fcp_init)
struct fcp_cmd {
  __u32 opcode;
  __u16 req_size;
  __u16 resp_size;
  __u8 data[];
} __attribute__((packed));
#define FCP_IOCTL_CMD _IOWR('S', 0x65, struct fcp_cmd)
struct fcp_meter_map {
  __u16 map_size;
  __u16 meter_slots;
  __s16 map[];
} __attribute__((packed));
#define FCP_IOCTL_SET_METER_MAP _IOW('S', 0x66, struct fcp_meter_map)
struct fcp_meter_labels {
  __u16 labels_size;
  char labels[];
} __attribute__((packed));
#define FCP_IOCTL_SET_METER_LABELS _IOW('S', 0x67, struct fcp_meter_labels)
#endif
