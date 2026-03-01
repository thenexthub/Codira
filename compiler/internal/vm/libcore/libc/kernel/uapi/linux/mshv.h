/*
 * This file is auto-generated. Modifications will be lost.
 *
 * See https://android.googlesource.com/platform/bionic/+/master/libc/kernel/
 * for more information.
 */
#ifndef _UAPI_LINUX_MSHV_H
#define _UAPI_LINUX_MSHV_H
#include <linux/types.h>
#define MSHV_IOCTL 0xB8
enum {
  MSHV_PT_BIT_LAPIC,
  MSHV_PT_BIT_X2APIC,
  MSHV_PT_BIT_GPA_SUPER_PAGES,
  MSHV_PT_BIT_COUNT,
};
#define MSHV_PT_FLAGS_MASK ((1 << MSHV_PT_BIT_COUNT) - 1)
enum {
  MSHV_PT_ISOLATION_NONE,
  MSHV_PT_ISOLATION_COUNT,
};
struct mshv_create_partition {
  __u64 pt_flags;
  __u64 pt_isolation;
};
#define MSHV_CREATE_PARTITION _IOW(MSHV_IOCTL, 0x00, struct mshv_create_partition)
struct mshv_create_vp {
  __u32 vp_index;
};
enum {
  MSHV_SET_MEM_BIT_WRITABLE,
  MSHV_SET_MEM_BIT_EXECUTABLE,
  MSHV_SET_MEM_BIT_UNMAP,
  MSHV_SET_MEM_BIT_COUNT
};
#define MSHV_SET_MEM_FLAGS_MASK ((1 << MSHV_SET_MEM_BIT_COUNT) - 1)
#define MSHV_HV_PAGE_SIZE 0x1000
struct mshv_user_mem_region {
  __u64 size;
  __u64 guest_pfn;
  __u64 userspace_addr;
  __u8 flags;
  __u8 rsvd[7];
};
enum {
  MSHV_IRQFD_BIT_DEASSIGN,
  MSHV_IRQFD_BIT_RESAMPLE,
  MSHV_IRQFD_BIT_COUNT,
};
#define MSHV_IRQFD_FLAGS_MASK ((1 << MSHV_IRQFD_BIT_COUNT) - 1)
struct mshv_user_irqfd {
  __s32 fd;
  __s32 resamplefd;
  __u32 gsi;
  __u32 flags;
};
enum {
  MSHV_IOEVENTFD_BIT_DATAMATCH,
  MSHV_IOEVENTFD_BIT_PIO,
  MSHV_IOEVENTFD_BIT_DEASSIGN,
  MSHV_IOEVENTFD_BIT_COUNT,
};
#define MSHV_IOEVENTFD_FLAGS_MASK ((1 << MSHV_IOEVENTFD_BIT_COUNT) - 1)
struct mshv_user_ioeventfd {
  __u64 datamatch;
  __u64 addr;
  __u32 len;
  __s32 fd;
  __u32 flags;
  __u8 rsvd[4];
};
struct mshv_user_irq_entry {
  __u32 gsi;
  __u32 address_lo;
  __u32 address_hi;
  __u32 data;
};
struct mshv_user_irq_table {
  __u32 nr;
  __u32 rsvd;
  struct mshv_user_irq_entry entries[];
};
enum {
  MSHV_GPAP_ACCESS_TYPE_ACCESSED,
  MSHV_GPAP_ACCESS_TYPE_DIRTY,
  MSHV_GPAP_ACCESS_TYPE_COUNT
};
enum {
  MSHV_GPAP_ACCESS_OP_NOOP,
  MSHV_GPAP_ACCESS_OP_CLEAR,
  MSHV_GPAP_ACCESS_OP_SET,
  MSHV_GPAP_ACCESS_OP_COUNT
};
struct mshv_gpap_access_bitmap {
  __u8 access_type;
  __u8 access_op;
  __u8 rsvd[6];
  __u64 page_count;
  __u64 gpap_base;
  __u64 bitmap_ptr;
};
struct mshv_root_hvcall {
  __u16 code;
  __u16 reps;
  __u16 in_sz;
  __u16 out_sz;
  __u16 status;
  __u8 rsvd[6];
  __u64 in_ptr;
  __u64 out_ptr;
};
#define MSHV_INITIALIZE_PARTITION _IO(MSHV_IOCTL, 0x00)
#define MSHV_CREATE_VP _IOW(MSHV_IOCTL, 0x01, struct mshv_create_vp)
#define MSHV_SET_GUEST_MEMORY _IOW(MSHV_IOCTL, 0x02, struct mshv_user_mem_region)
#define MSHV_IRQFD _IOW(MSHV_IOCTL, 0x03, struct mshv_user_irqfd)
#define MSHV_IOEVENTFD _IOW(MSHV_IOCTL, 0x04, struct mshv_user_ioeventfd)
#define MSHV_SET_MSI_ROUTING _IOW(MSHV_IOCTL, 0x05, struct mshv_user_irq_table)
#define MSHV_GET_GPAP_ACCESS_BITMAP _IOWR(MSHV_IOCTL, 0x06, struct mshv_gpap_access_bitmap)
#define MSHV_ROOT_HVCALL _IOWR(MSHV_IOCTL, 0x07, struct mshv_root_hvcall)
#define MSHV_RUN_VP_BUF_SZ 256
enum {
  MSHV_VP_MMAP_OFFSET_REGISTERS,
  MSHV_VP_MMAP_OFFSET_INTERCEPT_MESSAGE,
  MSHV_VP_MMAP_OFFSET_GHCB,
  MSHV_VP_MMAP_OFFSET_COUNT
};
struct mshv_run_vp {
  __u8 msg_buf[MSHV_RUN_VP_BUF_SZ];
};
enum {
  MSHV_VP_STATE_LAPIC,
  MSHV_VP_STATE_XSAVE,
  MSHV_VP_STATE_SIMP,
  MSHV_VP_STATE_SIEFP,
  MSHV_VP_STATE_SYNTHETIC_TIMERS,
  MSHV_VP_STATE_COUNT,
};
struct mshv_get_set_vp_state {
  __u8 type;
  __u8 rsvd[3];
  __u32 buf_sz;
  __u64 buf_ptr;
};
#define MSHV_RUN_VP _IOR(MSHV_IOCTL, 0x00, struct mshv_run_vp)
#define MSHV_GET_VP_STATE _IOWR(MSHV_IOCTL, 0x01, struct mshv_get_set_vp_state)
#define MSHV_SET_VP_STATE _IOWR(MSHV_IOCTL, 0x02, struct mshv_get_set_vp_state)
#endif
