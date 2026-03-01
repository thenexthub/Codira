/*
 * This file is auto-generated. Modifications will be lost.
 *
 * See https://android.googlesource.com/platform/bionic/+/master/libc/kernel/
 * for more information.
 */
#ifndef _AMD_APML_H_
#define _AMD_APML_H_
#include <linux/types.h>
#define AMD_SBI_MB_DATA_SIZE 4
struct apml_mbox_msg {
  __u32 cmd;
  __u32 mb_in_out;
  __u32 fw_ret_code;
};
struct apml_cpuid_msg {
  __u64 cpu_in_out;
  __u32 fw_ret_code;
  __u32 pad;
};
struct apml_mcamsr_msg {
  __u64 mcamsr_in_out;
  __u32 fw_ret_code;
  __u32 pad;
};
struct apml_reg_xfer_msg {
  __u16 reg_addr;
  __u8 data_in_out;
  __u8 rflag;
};
#define SB_BASE_IOCTL_NR 0xF9
#define SBRMI_IOCTL_MBOX_CMD _IOWR(SB_BASE_IOCTL_NR, 0, struct apml_mbox_msg)
#define SBRMI_IOCTL_CPUID_CMD _IOWR(SB_BASE_IOCTL_NR, 1, struct apml_cpuid_msg)
#define SBRMI_IOCTL_MCAMSR_CMD _IOWR(SB_BASE_IOCTL_NR, 2, struct apml_mcamsr_msg)
#define SBRMI_IOCTL_REG_XFER_CMD _IOWR(SB_BASE_IOCTL_NR, 3, struct apml_reg_xfer_msg)
#endif
