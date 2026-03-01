/*
 * This file is auto-generated. Modifications will be lost.
 *
 * See https://android.googlesource.com/platform/bionic/+/master/libc/kernel/
 * for more information.
 */
#ifndef _LINUX_VIRTIO_RTC_H
#define _LINUX_VIRTIO_RTC_H
#include <linux/types.h>
#define VIRTIO_RTC_F_ALARM 0
#define VIRTIO_RTC_REQ_READ 0x0001
#define VIRTIO_RTC_REQ_READ_CROSS 0x0002
#define VIRTIO_RTC_REQ_CFG 0x1000
#define VIRTIO_RTC_REQ_CLOCK_CAP 0x1001
#define VIRTIO_RTC_REQ_CROSS_CAP 0x1002
#define VIRTIO_RTC_REQ_READ_ALARM 0x1003
#define VIRTIO_RTC_REQ_SET_ALARM 0x1004
#define VIRTIO_RTC_REQ_SET_ALARM_ENABLED 0x1005
#define VIRTIO_RTC_NOTIF_ALARM 0x2000
struct virtio_rtc_req_head {
  __le16 msg_type;
  __u8 reserved[6];
};
struct virtio_rtc_resp_head {
#define VIRTIO_RTC_S_OK 0
#define VIRTIO_RTC_S_EOPNOTSUPP 2
#define VIRTIO_RTC_S_ENODEV 3
#define VIRTIO_RTC_S_EINVAL 4
#define VIRTIO_RTC_S_EIO 5
  __u8 status;
  __u8 reserved[7];
};
struct virtio_rtc_notif_head {
  __le16 msg_type;
  __u8 reserved[6];
};
struct virtio_rtc_req_read {
  struct virtio_rtc_req_head head;
  __le16 clock_id;
  __u8 reserved[6];
};
struct virtio_rtc_resp_read {
  struct virtio_rtc_resp_head head;
  __le64 clock_reading;
};
struct virtio_rtc_req_read_cross {
  struct virtio_rtc_req_head head;
  __le16 clock_id;
#define VIRTIO_RTC_COUNTER_ARM_VCT 0
#define VIRTIO_RTC_COUNTER_X86_TSC 1
#define VIRTIO_RTC_COUNTER_INVALID 0xFF
  __u8 hw_counter;
  __u8 reserved[5];
};
struct virtio_rtc_resp_read_cross {
  struct virtio_rtc_resp_head head;
  __le64 clock_reading;
  __le64 counter_cycles;
};
struct virtio_rtc_req_cfg {
  struct virtio_rtc_req_head head;
};
struct virtio_rtc_resp_cfg {
  struct virtio_rtc_resp_head head;
  __le16 num_clocks;
  __u8 reserved[6];
};
struct virtio_rtc_req_clock_cap {
  struct virtio_rtc_req_head head;
  __le16 clock_id;
  __u8 reserved[6];
};
struct virtio_rtc_resp_clock_cap {
  struct virtio_rtc_resp_head head;
#define VIRTIO_RTC_CLOCK_UTC 0
#define VIRTIO_RTC_CLOCK_TAI 1
#define VIRTIO_RTC_CLOCK_MONOTONIC 2
#define VIRTIO_RTC_CLOCK_UTC_SMEARED 3
#define VIRTIO_RTC_CLOCK_UTC_MAYBE_SMEARED 4
  __u8 type;
#define VIRTIO_RTC_SMEAR_UNSPECIFIED 0
#define VIRTIO_RTC_SMEAR_NOON_LINEAR 1
#define VIRTIO_RTC_SMEAR_UTC_SLS 2
  __u8 leap_second_smearing;
#define VIRTIO_RTC_FLAG_ALARM_CAP (1 << 0)
  __u8 flags;
  __u8 reserved[5];
};
struct virtio_rtc_req_cross_cap {
  struct virtio_rtc_req_head head;
  __le16 clock_id;
  __u8 hw_counter;
  __u8 reserved[5];
};
struct virtio_rtc_resp_cross_cap {
  struct virtio_rtc_resp_head head;
#define VIRTIO_RTC_FLAG_CROSS_CAP (1 << 0)
  __u8 flags;
  __u8 reserved[7];
};
struct virtio_rtc_req_read_alarm {
  struct virtio_rtc_req_head head;
  __le16 clock_id;
  __u8 reserved[6];
};
struct virtio_rtc_resp_read_alarm {
  struct virtio_rtc_resp_head head;
  __le64 alarm_time;
#define VIRTIO_RTC_FLAG_ALARM_ENABLED (1 << 0)
  __u8 flags;
  __u8 reserved[7];
};
struct virtio_rtc_req_set_alarm {
  struct virtio_rtc_req_head head;
  __le64 alarm_time;
  __le16 clock_id;
  __u8 flags;
  __u8 reserved[5];
};
struct virtio_rtc_resp_set_alarm {
  struct virtio_rtc_resp_head head;
};
struct virtio_rtc_req_set_alarm_enabled {
  struct virtio_rtc_req_head head;
  __le16 clock_id;
  __u8 flags;
  __u8 reserved[5];
};
struct virtio_rtc_resp_set_alarm_enabled {
  struct virtio_rtc_resp_head head;
};
union virtio_rtc_req_requestq {
  struct virtio_rtc_req_read read;
  struct virtio_rtc_req_read_cross read_cross;
  struct virtio_rtc_req_cfg cfg;
  struct virtio_rtc_req_clock_cap clock_cap;
  struct virtio_rtc_req_cross_cap cross_cap;
  struct virtio_rtc_req_read_alarm read_alarm;
  struct virtio_rtc_req_set_alarm set_alarm;
  struct virtio_rtc_req_set_alarm_enabled set_alarm_enabled;
};
union virtio_rtc_resp_requestq {
  struct virtio_rtc_resp_read read;
  struct virtio_rtc_resp_read_cross read_cross;
  struct virtio_rtc_resp_cfg cfg;
  struct virtio_rtc_resp_clock_cap clock_cap;
  struct virtio_rtc_resp_cross_cap cross_cap;
  struct virtio_rtc_resp_read_alarm read_alarm;
  struct virtio_rtc_resp_set_alarm set_alarm;
  struct virtio_rtc_resp_set_alarm_enabled set_alarm_enabled;
};
struct virtio_rtc_notif_alarm {
  struct virtio_rtc_notif_head head;
  __le16 clock_id;
  __u8 reserved[6];
};
union virtio_rtc_notif_alarmq {
  struct virtio_rtc_notif_alarm alarm;
};
#endif
