/*
 * This file is auto-generated. Modifications will be lost.
 *
 * See https://android.googlesource.com/platform/bionic/+/master/libc/kernel/
 * for more information.
 */
#ifndef _UAPI_FWCTL_PDS_H_
#define _UAPI_FWCTL_PDS_H_
#include <linux/types.h>
struct fwctl_info_pds {
  __u32 uctx_caps;
};
enum pds_fwctl_capabilities {
  PDS_FWCTL_QUERY_CAP = 0,
  PDS_FWCTL_SEND_CAP,
};
struct fwctl_rpc_pds {
  struct {
    __u32 op;
    __u32 ep;
    __u32 rsvd;
    __u32 len;
    __aligned_u64 payload;
  } in;
  struct {
    __u32 retval;
    __u32 rsvd[2];
    __u32 len;
    __aligned_u64 payload;
  } out;
};
#endif
