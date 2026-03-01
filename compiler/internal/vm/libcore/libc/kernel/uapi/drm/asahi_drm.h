/*
 * This file is auto-generated. Modifications will be lost.
 *
 * See https://android.googlesource.com/platform/bionic/+/master/libc/kernel/
 * for more information.
 */
#ifndef _ASAHI_DRM_H_
#define _ASAHI_DRM_H_
#include "drm.h"
#ifdef __cplusplus
extern "C" {
#endif
enum drm_asahi_ioctl_id {
  DRM_ASAHI_GET_PARAMS = 0,
  DRM_ASAHI_GET_TIME,
  DRM_ASAHI_VM_CREATE,
  DRM_ASAHI_VM_DESTROY,
  DRM_ASAHI_VM_BIND,
  DRM_ASAHI_GEM_CREATE,
  DRM_ASAHI_GEM_MMAP_OFFSET,
  DRM_ASAHI_GEM_BIND_OBJECT,
  DRM_ASAHI_QUEUE_CREATE,
  DRM_ASAHI_QUEUE_DESTROY,
  DRM_ASAHI_SUBMIT,
};
#define DRM_ASAHI_MAX_CLUSTERS 64
struct drm_asahi_params_global {
  __u64 features;
  __u32 gpu_generation;
  __u32 gpu_variant;
  __u32 gpu_revision;
  __u32 chip_id;
  __u32 num_dies;
  __u32 num_clusters_total;
  __u32 num_cores_per_cluster;
  __u32 max_frequency_khz;
  __u64 core_masks[DRM_ASAHI_MAX_CLUSTERS];
  __u64 vm_start;
  __u64 vm_end;
  __u64 vm_kernel_min_size;
  __u32 max_commands_per_submission;
  __u32 max_attachments;
  __u64 command_timestamp_frequency_hz;
};
enum drm_asahi_feature {
  DRM_ASAHI_FEATURE_SOFT_FAULTS = (1UL) << 0,
};
struct drm_asahi_get_params {
  __u32 param_group;
  __u32 pad;
  __u64 pointer;
  __u64 size;
};
struct drm_asahi_vm_create {
  __u64 kernel_start;
  __u64 kernel_end;
  __u32 vm_id;
  __u32 pad;
};
struct drm_asahi_vm_destroy {
  __u32 vm_id;
  __u32 pad;
};
enum drm_asahi_gem_flags {
  DRM_ASAHI_GEM_WRITEBACK = (1L << 0),
  DRM_ASAHI_GEM_VM_PRIVATE = (1L << 1),
};
struct drm_asahi_gem_create {
  __u64 size;
  __u32 flags;
  __u32 vm_id;
  __u32 handle;
  __u32 pad;
};
struct drm_asahi_gem_mmap_offset {
  __u32 handle;
  __u32 flags;
  __u64 offset;
};
enum drm_asahi_bind_flags {
  DRM_ASAHI_BIND_UNBIND = (1L << 0),
  DRM_ASAHI_BIND_READ = (1L << 1),
  DRM_ASAHI_BIND_WRITE = (1L << 2),
  DRM_ASAHI_BIND_SINGLE_PAGE = (1L << 3),
};
struct drm_asahi_gem_bind_op {
  __u32 flags;
  __u32 handle;
  __u64 offset;
  __u64 range;
  __u64 addr;
};
struct drm_asahi_vm_bind {
  __u32 vm_id;
  __u32 num_binds;
  __u32 stride;
  __u32 pad;
  __u64 userptr;
};
enum drm_asahi_bind_object_op {
  DRM_ASAHI_BIND_OBJECT_OP_BIND = 0,
  DRM_ASAHI_BIND_OBJECT_OP_UNBIND = 1,
};
enum drm_asahi_bind_object_flags {
  DRM_ASAHI_BIND_OBJECT_USAGE_TIMESTAMPS = (1L << 0),
};
struct drm_asahi_gem_bind_object {
  __u32 op;
  __u32 flags;
  __u32 handle;
  __u32 vm_id;
  __u64 offset;
  __u64 range;
  __u32 object_handle;
  __u32 pad;
};
enum drm_asahi_cmd_type {
  DRM_ASAHI_CMD_RENDER = 0,
  DRM_ASAHI_CMD_COMPUTE = 1,
  DRM_ASAHI_SET_VERTEX_ATTACHMENTS = 2,
  DRM_ASAHI_SET_FRAGMENT_ATTACHMENTS = 3,
  DRM_ASAHI_SET_COMPUTE_ATTACHMENTS = 4,
};
enum drm_asahi_priority {
  DRM_ASAHI_PRIORITY_LOW = 0,
  DRM_ASAHI_PRIORITY_MEDIUM = 1,
  DRM_ASAHI_PRIORITY_HIGH = 2,
  DRM_ASAHI_PRIORITY_REALTIME = 3,
};
struct drm_asahi_queue_create {
  __u32 flags;
  __u32 vm_id;
  __u32 priority;
  __u32 queue_id;
  __u64 usc_exec_base;
};
struct drm_asahi_queue_destroy {
  __u32 queue_id;
  __u32 pad;
};
enum drm_asahi_sync_type {
  DRM_ASAHI_SYNC_SYNCOBJ = 0,
  DRM_ASAHI_SYNC_TIMELINE_SYNCOBJ = 1,
};
struct drm_asahi_sync {
  __u32 sync_type;
  __u32 handle;
  __u64 timeline_value;
};
#define DRM_ASAHI_BARRIER_NONE (0xFFFFu)
struct drm_asahi_cmd_header {
  __u16 cmd_type;
  __u16 size;
  __u16 vdm_barrier;
  __u16 cdm_barrier;
};
struct drm_asahi_submit {
  __u64 syncs;
  __u64 cmdbuf;
  __u32 flags;
  __u32 queue_id;
  __u32 in_sync_count;
  __u32 out_sync_count;
  __u32 cmdbuf_size;
  __u32 pad;
};
struct drm_asahi_attachment {
  __u64 pointer;
  __u64 size;
  __u32 pad;
  __u32 flags;
};
enum drm_asahi_render_flags {
  DRM_ASAHI_RENDER_VERTEX_SCRATCH = (1U << 0),
  DRM_ASAHI_RENDER_PROCESS_EMPTY_TILES = (1U << 1),
  DRM_ASAHI_RENDER_NO_VERTEX_CLUSTERING = (1U << 2),
  DRM_ASAHI_RENDER_DBIAS_IS_INT = (1U << 18),
};
struct drm_asahi_zls_buffer {
  __u64 base;
  __u64 comp_base;
  __u32 stride;
  __u32 comp_stride;
};
struct drm_asahi_timestamp {
  __u32 handle;
  __u32 offset;
};
struct drm_asahi_timestamps {
  struct drm_asahi_timestamp start;
  struct drm_asahi_timestamp end;
};
struct drm_asahi_helper_program {
  __u32 binary;
  __u32 cfg;
  __u64 data;
};
struct drm_asahi_bg_eot {
  __u32 usc;
  __u32 rsrc_spec;
};
struct drm_asahi_cmd_render {
  __u32 flags;
  __u32 isp_zls_pixels;
  __u64 vdm_ctrl_stream_base;
  struct drm_asahi_helper_program vertex_helper;
  struct drm_asahi_helper_program fragment_helper;
  __u64 isp_scissor_base;
  __u64 isp_dbias_base;
  __u64 isp_oclqry_base;
  struct drm_asahi_zls_buffer depth;
  struct drm_asahi_zls_buffer stencil;
  __u64 zls_ctrl;
  __u64 ppp_multisamplectl;
  __u64 sampler_heap;
  __u32 ppp_ctrl;
  __u16 width_px;
  __u16 height_px;
  __u16 layers;
  __u16 sampler_count;
  __u8 utile_width_px;
  __u8 utile_height_px;
  __u8 samples;
  __u8 sample_size_B;
  __u32 isp_merge_upper_x;
  __u32 isp_merge_upper_y;
  struct drm_asahi_bg_eot bg;
  struct drm_asahi_bg_eot eot;
  struct drm_asahi_bg_eot partial_bg;
  struct drm_asahi_bg_eot partial_eot;
  __u32 isp_bgobjdepth;
  __u32 isp_bgobjvals;
  struct drm_asahi_timestamps ts_vtx;
  struct drm_asahi_timestamps ts_frag;
};
struct drm_asahi_cmd_compute {
  __u32 flags;
  __u32 sampler_count;
  __u64 cdm_ctrl_stream_base;
  __u64 cdm_ctrl_stream_end;
  __u64 sampler_heap;
  struct drm_asahi_helper_program helper;
  struct drm_asahi_timestamps ts;
};
struct drm_asahi_get_time {
  __u64 flags;
  __u64 gpu_timestamp;
};
#define DRM_IOCTL_ASAHI(__access,__id,__type) DRM_IO ##__access(DRM_COMMAND_BASE + DRM_ASAHI_ ##__id, struct drm_asahi_ ##__type)
enum {
  DRM_IOCTL_ASAHI_GET_PARAMS = DRM_IOCTL_ASAHI(W, GET_PARAMS, get_params),
  DRM_IOCTL_ASAHI_GET_TIME = DRM_IOCTL_ASAHI(WR, GET_TIME, get_time),
  DRM_IOCTL_ASAHI_VM_CREATE = DRM_IOCTL_ASAHI(WR, VM_CREATE, vm_create),
  DRM_IOCTL_ASAHI_VM_DESTROY = DRM_IOCTL_ASAHI(W, VM_DESTROY, vm_destroy),
  DRM_IOCTL_ASAHI_VM_BIND = DRM_IOCTL_ASAHI(W, VM_BIND, vm_bind),
  DRM_IOCTL_ASAHI_GEM_CREATE = DRM_IOCTL_ASAHI(WR, GEM_CREATE, gem_create),
  DRM_IOCTL_ASAHI_GEM_MMAP_OFFSET = DRM_IOCTL_ASAHI(WR, GEM_MMAP_OFFSET, gem_mmap_offset),
  DRM_IOCTL_ASAHI_GEM_BIND_OBJECT = DRM_IOCTL_ASAHI(WR, GEM_BIND_OBJECT, gem_bind_object),
  DRM_IOCTL_ASAHI_QUEUE_CREATE = DRM_IOCTL_ASAHI(WR, QUEUE_CREATE, queue_create),
  DRM_IOCTL_ASAHI_QUEUE_DESTROY = DRM_IOCTL_ASAHI(W, QUEUE_DESTROY, queue_destroy),
  DRM_IOCTL_ASAHI_SUBMIT = DRM_IOCTL_ASAHI(W, SUBMIT, submit),
};
#ifdef __cplusplus
}
#endif
#endif
