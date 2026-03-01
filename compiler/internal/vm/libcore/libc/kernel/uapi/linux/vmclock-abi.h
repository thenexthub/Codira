/*
 * This file is auto-generated. Modifications will be lost.
 *
 * See https://android.googlesource.com/platform/bionic/+/master/libc/kernel/
 * for more information.
 */
#ifndef __VMCLOCK_ABI_H__
#define __VMCLOCK_ABI_H__
#include <linux/types.h>
struct vmclock_abi {
  __le32 magic;
#define VMCLOCK_MAGIC 0x4b4c4356
  __le32 size;
  __le16 version;
  __u8 counter_id;
#define VMCLOCK_COUNTER_ARM_VCNT 0
#define VMCLOCK_COUNTER_X86_TSC 1
#define VMCLOCK_COUNTER_INVALID 0xff
  __u8 time_type;
#define VMCLOCK_TIME_UTC 0
#define VMCLOCK_TIME_TAI 1
#define VMCLOCK_TIME_MONOTONIC 2
#define VMCLOCK_TIME_INVALID_SMEARED 3
#define VMCLOCK_TIME_INVALID_MAYBE_SMEARED 4
  __le32 seq_count;
  __le64 disruption_marker;
  __le64 flags;
#define VMCLOCK_FLAG_TAI_OFFSET_VALID (1 << 0)
#define VMCLOCK_FLAG_DISRUPTION_SOON (1 << 1)
#define VMCLOCK_FLAG_DISRUPTION_IMMINENT (1 << 2)
#define VMCLOCK_FLAG_PERIOD_ESTERROR_VALID (1 << 3)
#define VMCLOCK_FLAG_PERIOD_MAXERROR_VALID (1 << 4)
#define VMCLOCK_FLAG_TIME_ESTERROR_VALID (1 << 5)
#define VMCLOCK_FLAG_TIME_MAXERROR_VALID (1 << 6)
#define VMCLOCK_FLAG_TIME_MONOTONIC (1 << 7)
  __u8 pad[2];
  __u8 clock_status;
#define VMCLOCK_STATUS_UNKNOWN 0
#define VMCLOCK_STATUS_INITIALIZING 1
#define VMCLOCK_STATUS_SYNCHRONIZED 2
#define VMCLOCK_STATUS_FREERUNNING 3
#define VMCLOCK_STATUS_UNRELIABLE 4
  __u8 leap_second_smearing_hint;
#define VMCLOCK_SMEARING_STRICT 0
#define VMCLOCK_SMEARING_NOON_LINEAR 1
#define VMCLOCK_SMEARING_UTC_SLS 2
  __le16 tai_offset_sec;
  __u8 leap_indicator;
#define VMCLOCK_LEAP_NONE 0x00
#define VMCLOCK_LEAP_PRE_POS 0x01
#define VMCLOCK_LEAP_PRE_NEG 0x02
#define VMCLOCK_LEAP_POS 0x03
#define VMCLOCK_LEAP_POST_POS 0x04
#define VMCLOCK_LEAP_POST_NEG 0x05
  __u8 counter_period_shift;
  __le64 counter_value;
  __le64 counter_period_frac_sec;
  __le64 counter_period_esterror_rate_frac_sec;
  __le64 counter_period_maxerror_rate_frac_sec;
  __le64 time_sec;
  __le64 time_frac_sec;
  __le64 time_esterror_nanosec;
  __le64 time_maxerror_nanosec;
};
#endif
