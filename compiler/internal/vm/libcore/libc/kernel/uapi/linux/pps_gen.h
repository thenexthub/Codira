/*
 * This file is auto-generated. Modifications will be lost.
 *
 * See https://android.googlesource.com/platform/bionic/+/master/libc/kernel/
 * for more information.
 */
#ifndef _PPS_GEN_H_
#define _PPS_GEN_H_
#include <linux/types.h>
#include <linux/ioctl.h>
struct pps_gen_event {
  unsigned int event;
  unsigned int sequence;
};
#define PPS_GEN_EVENT_MISSEDPULSE 1
#define PPS_GEN_SETENABLE _IOW('p', 0xb1, unsigned int *)
#define PPS_GEN_USESYSTEMCLOCK _IOR('p', 0xb2, unsigned int *)
#define PPS_GEN_FETCHEVENT _IOR('p', 0xb3, struct pps_gen_event *)
#endif
