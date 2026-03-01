/*
 * This file is auto-generated. Modifications will be lost.
 *
 * See https://android.googlesource.com/platform/bionic/+/master/libc/kernel/
 * for more information.
 */
#ifndef _UAPI_LINUX_PIDFD_H
#define _UAPI_LINUX_PIDFD_H
#include <linux/types.h>
#include <linux/fcntl.h>
#include <linux/ioctl.h>
#define PIDFD_NONBLOCK O_NONBLOCK
#define PIDFD_THREAD O_EXCL
#define PIDFD_SIGNAL_THREAD (1UL << 0)
#define PIDFD_SIGNAL_THREAD_GROUP (1UL << 1)
#define PIDFD_SIGNAL_PROCESS_GROUP (1UL << 2)
#define PIDFD_INFO_PID (1UL << 0)
#define PIDFD_INFO_CREDS (1UL << 1)
#define PIDFD_INFO_CGROUPID (1UL << 2)
#define PIDFD_INFO_EXIT (1UL << 3)
#define PIDFD_INFO_COREDUMP (1UL << 4)
#define PIDFD_INFO_SIZE_VER0 64
#define PIDFD_COREDUMPED (1U << 0)
#define PIDFD_COREDUMP_SKIP (1U << 1)
#define PIDFD_COREDUMP_USER (1U << 2)
#define PIDFD_COREDUMP_ROOT (1U << 3)
#define PIDFD_SELF_THREAD - 10000
#define PIDFD_SELF_THREAD_GROUP - 20000
#define PIDFD_SELF PIDFD_SELF_THREAD
#define PIDFD_SELF_PROCESS PIDFD_SELF_THREAD_GROUP
struct pidfd_info {
  __u64 mask;
  __u64 cgroupid;
  __u32 pid;
  __u32 tgid;
  __u32 ppid;
  __u32 ruid;
  __u32 rgid;
  __u32 euid;
  __u32 egid;
  __u32 suid;
  __u32 sgid;
  __u32 fsuid;
  __u32 fsgid;
  __s32 exit_code;
  __u32 coredump_mask;
  __u32 __spare1;
};
#define PIDFS_IOCTL_MAGIC 0xFF
#define PIDFD_GET_CGROUP_NAMESPACE _IO(PIDFS_IOCTL_MAGIC, 1)
#define PIDFD_GET_IPC_NAMESPACE _IO(PIDFS_IOCTL_MAGIC, 2)
#define PIDFD_GET_MNT_NAMESPACE _IO(PIDFS_IOCTL_MAGIC, 3)
#define PIDFD_GET_NET_NAMESPACE _IO(PIDFS_IOCTL_MAGIC, 4)
#define PIDFD_GET_PID_NAMESPACE _IO(PIDFS_IOCTL_MAGIC, 5)
#define PIDFD_GET_PID_FOR_CHILDREN_NAMESPACE _IO(PIDFS_IOCTL_MAGIC, 6)
#define PIDFD_GET_TIME_NAMESPACE _IO(PIDFS_IOCTL_MAGIC, 7)
#define PIDFD_GET_TIME_FOR_CHILDREN_NAMESPACE _IO(PIDFS_IOCTL_MAGIC, 8)
#define PIDFD_GET_USER_NAMESPACE _IO(PIDFS_IOCTL_MAGIC, 9)
#define PIDFD_GET_UTS_NAMESPACE _IO(PIDFS_IOCTL_MAGIC, 10)
#define PIDFD_GET_INFO _IOWR(PIDFS_IOCTL_MAGIC, 11, struct pidfd_info)
#endif
