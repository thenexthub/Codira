/*
 * This file is auto-generated. Modifications will be lost.
 *
 * See https://android.googlesource.com/platform/bionic/+/master/libc/kernel/
 * for more information.
 */
#ifndef _UAPI_LINUX_LOCKD_NETLINK_H
#define _UAPI_LINUX_LOCKD_NETLINK_H
#define LOCKD_FAMILY_NAME "lockd"
#define LOCKD_FAMILY_VERSION 1
enum {
  LOCKD_A_SERVER_GRACETIME = 1,
  LOCKD_A_SERVER_TCP_PORT,
  LOCKD_A_SERVER_UDP_PORT,
  __LOCKD_A_SERVER_MAX,
  LOCKD_A_SERVER_MAX = (__LOCKD_A_SERVER_MAX - 1)
};
enum {
  LOCKD_CMD_SERVER_SET = 1,
  LOCKD_CMD_SERVER_GET,
  __LOCKD_CMD_MAX,
  LOCKD_CMD_MAX = (__LOCKD_CMD_MAX - 1)
};
#endif
