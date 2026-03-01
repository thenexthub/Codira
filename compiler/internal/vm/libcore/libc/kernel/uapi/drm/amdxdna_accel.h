/*
 * This file is auto-generated. Modifications will be lost.
 *
 * See https://android.googlesource.com/platform/bionic/+/master/libc/kernel/
 * for more information.
 */
#ifndef _UAPI_AMDXDNA_ACCEL_H_
#define _UAPI_AMDXDNA_ACCEL_H_
#include <linux/stddef.h>
#include "drm.h"
#ifdef __cplusplus
extern "C" {
#endif
#define AMDXDNA_INVALID_CMD_HANDLE (~0UL)
#define AMDXDNA_INVALID_ADDR (~0UL)
#define AMDXDNA_INVALID_CTX_HANDLE 0
#define AMDXDNA_INVALID_BO_HANDLE 0
#define AMDXDNA_INVALID_FENCE_HANDLE 0
enum amdxdna_device_type {
  AMDXDNA_DEV_TYPE_UNKNOWN = - 1,
  AMDXDNA_DEV_TYPE_KMQ,
};
enum amdxdna_drm_ioctl_id {
  DRM_AMDXDNA_CREATE_HWCTX,
  DRM_AMDXDNA_DESTROY_HWCTX,
  DRM_AMDXDNA_CONFIG_HWCTX,
  DRM_AMDXDNA_CREATE_BO,
  DRM_AMDXDNA_GET_BO_INFO,
  DRM_AMDXDNA_SYNC_BO,
  DRM_AMDXDNA_EXEC_CMD,
  DRM_AMDXDNA_GET_INFO,
  DRM_AMDXDNA_SET_STATE,
};
struct amdxdna_qos_info {
  __u32 gops;
  __u32 fps;
  __u32 dma_bandwidth;
  __u32 latency;
  __u32 frame_exec_time;
  __u32 priority;
};
struct amdxdna_drm_create_hwctx {
  __u64 ext;
  __u64 ext_flags;
  __u64 qos_p;
  __u32 umq_bo;
  __u32 log_buf_bo;
  __u32 max_opc;
  __u32 num_tiles;
  __u32 mem_size;
  __u32 umq_doorbell;
  __u32 handle;
  __u32 syncobj_handle;
};
struct amdxdna_drm_destroy_hwctx {
  __u32 handle;
  __u32 pad;
};
struct amdxdna_cu_config {
  __u32 cu_bo;
  __u8 cu_func;
  __u8 pad[3];
};
struct amdxdna_hwctx_param_config_cu {
  __u16 num_cus;
  __u16 pad[3];
  struct amdxdna_cu_config cu_configs[] __counted_by(num_cus);
};
enum amdxdna_drm_config_hwctx_param {
  DRM_AMDXDNA_HWCTX_CONFIG_CU,
  DRM_AMDXDNA_HWCTX_ASSIGN_DBG_BUF,
  DRM_AMDXDNA_HWCTX_REMOVE_DBG_BUF,
};
struct amdxdna_drm_config_hwctx {
  __u32 handle;
  __u32 param_type;
  __u64 param_val;
  __u32 param_val_size;
  __u32 pad;
};
enum amdxdna_bo_type {
  AMDXDNA_BO_INVALID = 0,
  AMDXDNA_BO_SHMEM,
  AMDXDNA_BO_DEV_HEAP,
  AMDXDNA_BO_DEV,
  AMDXDNA_BO_CMD,
};
struct amdxdna_drm_create_bo {
  __u64 flags;
  __u64 vaddr;
  __u64 size;
  __u32 type;
  __u32 handle;
};
struct amdxdna_drm_get_bo_info {
  __u64 ext;
  __u64 ext_flags;
  __u32 handle;
  __u32 pad;
  __u64 map_offset;
  __u64 vaddr;
  __u64 xdna_addr;
};
struct amdxdna_drm_sync_bo {
  __u32 handle;
#define SYNC_DIRECT_TO_DEVICE 0U
#define SYNC_DIRECT_FROM_DEVICE 1U
  __u32 direction;
  __u64 offset;
  __u64 size;
};
enum amdxdna_cmd_type {
  AMDXDNA_CMD_SUBMIT_EXEC_BUF = 0,
  AMDXDNA_CMD_SUBMIT_DEPENDENCY,
  AMDXDNA_CMD_SUBMIT_SIGNAL,
};
struct amdxdna_drm_exec_cmd {
  __u64 ext;
  __u64 ext_flags;
  __u32 hwctx;
  __u32 type;
  __u64 cmd_handles;
  __u64 args;
  __u32 cmd_count;
  __u32 arg_count;
  __u64 seq;
};
struct amdxdna_drm_query_aie_status {
  __u64 buffer;
  __u32 buffer_size;
  __u32 cols_filled;
};
struct amdxdna_drm_query_aie_version {
  __u32 major;
  __u32 minor;
};
struct amdxdna_drm_query_aie_tile_metadata {
  __u16 row_count;
  __u16 row_start;
  __u16 dma_channel_count;
  __u16 lock_count;
  __u16 event_reg_count;
  __u16 pad[3];
};
struct amdxdna_drm_query_aie_metadata {
  __u32 col_size;
  __u16 cols;
  __u16 rows;
  struct amdxdna_drm_query_aie_version version;
  struct amdxdna_drm_query_aie_tile_metadata core;
  struct amdxdna_drm_query_aie_tile_metadata mem;
  struct amdxdna_drm_query_aie_tile_metadata shim;
};
struct amdxdna_drm_query_clock {
  __u8 name[16];
  __u32 freq_mhz;
  __u32 pad;
};
struct amdxdna_drm_query_clock_metadata {
  struct amdxdna_drm_query_clock mp_npu_clock;
  struct amdxdna_drm_query_clock h_clock;
};
enum amdxdna_sensor_type {
  AMDXDNA_SENSOR_TYPE_POWER
};
struct amdxdna_drm_query_sensor {
  __u8 label[64];
  __u32 input;
  __u32 max;
  __u32 average;
  __u32 highest;
  __u8 status[64];
  __u8 units[16];
  __s8 unitm;
  __u8 type;
  __u8 pad[6];
};
struct amdxdna_drm_query_hwctx {
  __u32 context_id;
  __u32 start_col;
  __u32 num_col;
  __u32 pad;
  __s64 pid;
  __u64 command_submissions;
  __u64 command_completions;
  __u64 migrations;
  __u64 preemptions;
  __u64 errors;
};
enum amdxdna_power_mode_type {
  POWER_MODE_DEFAULT,
  POWER_MODE_LOW,
  POWER_MODE_MEDIUM,
  POWER_MODE_HIGH,
  POWER_MODE_TURBO,
};
struct amdxdna_drm_get_power_mode {
  __u8 power_mode;
  __u8 pad[7];
};
struct amdxdna_drm_query_firmware_version {
  __u32 major;
  __u32 minor;
  __u32 patch;
  __u32 build;
};
enum amdxdna_drm_get_param {
  DRM_AMDXDNA_QUERY_AIE_STATUS,
  DRM_AMDXDNA_QUERY_AIE_METADATA,
  DRM_AMDXDNA_QUERY_AIE_VERSION,
  DRM_AMDXDNA_QUERY_CLOCK_METADATA,
  DRM_AMDXDNA_QUERY_SENSORS,
  DRM_AMDXDNA_QUERY_HW_CONTEXTS,
  DRM_AMDXDNA_QUERY_FIRMWARE_VERSION = 8,
  DRM_AMDXDNA_GET_POWER_MODE,
};
struct amdxdna_drm_get_info {
  __u32 param;
  __u32 buffer_size;
  __u64 buffer;
};
enum amdxdna_drm_set_param {
  DRM_AMDXDNA_SET_POWER_MODE,
  DRM_AMDXDNA_WRITE_AIE_MEM,
  DRM_AMDXDNA_WRITE_AIE_REG,
};
struct amdxdna_drm_set_state {
  __u32 param;
  __u32 buffer_size;
  __u64 buffer;
};
struct amdxdna_drm_set_power_mode {
  __u8 power_mode;
  __u8 pad[7];
};
#define DRM_IOCTL_AMDXDNA_CREATE_HWCTX DRM_IOWR(DRM_COMMAND_BASE + DRM_AMDXDNA_CREATE_HWCTX, struct amdxdna_drm_create_hwctx)
#define DRM_IOCTL_AMDXDNA_DESTROY_HWCTX DRM_IOWR(DRM_COMMAND_BASE + DRM_AMDXDNA_DESTROY_HWCTX, struct amdxdna_drm_destroy_hwctx)
#define DRM_IOCTL_AMDXDNA_CONFIG_HWCTX DRM_IOWR(DRM_COMMAND_BASE + DRM_AMDXDNA_CONFIG_HWCTX, struct amdxdna_drm_config_hwctx)
#define DRM_IOCTL_AMDXDNA_CREATE_BO DRM_IOWR(DRM_COMMAND_BASE + DRM_AMDXDNA_CREATE_BO, struct amdxdna_drm_create_bo)
#define DRM_IOCTL_AMDXDNA_GET_BO_INFO DRM_IOWR(DRM_COMMAND_BASE + DRM_AMDXDNA_GET_BO_INFO, struct amdxdna_drm_get_bo_info)
#define DRM_IOCTL_AMDXDNA_SYNC_BO DRM_IOWR(DRM_COMMAND_BASE + DRM_AMDXDNA_SYNC_BO, struct amdxdna_drm_sync_bo)
#define DRM_IOCTL_AMDXDNA_EXEC_CMD DRM_IOWR(DRM_COMMAND_BASE + DRM_AMDXDNA_EXEC_CMD, struct amdxdna_drm_exec_cmd)
#define DRM_IOCTL_AMDXDNA_GET_INFO DRM_IOWR(DRM_COMMAND_BASE + DRM_AMDXDNA_GET_INFO, struct amdxdna_drm_get_info)
#define DRM_IOCTL_AMDXDNA_SET_STATE DRM_IOWR(DRM_COMMAND_BASE + DRM_AMDXDNA_SET_STATE, struct amdxdna_drm_set_state)
#ifdef __cplusplus
}
#endif
#endif
