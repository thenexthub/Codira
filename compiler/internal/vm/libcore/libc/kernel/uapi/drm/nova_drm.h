/*
 * This file is auto-generated. Modifications will be lost.
 *
 * See https://android.googlesource.com/platform/bionic/+/master/libc/kernel/
 * for more information.
 */
#ifndef __NOVA_DRM_H__
#define __NOVA_DRM_H__
#include "drm.h"
#ifdef __cplusplus
extern "C" {
#endif
#define NOVA_GETPARAM_VRAM_BAR_SIZE 0x1
struct drm_nova_getparam {
  __u64 param;
  __u64 value;
};
struct drm_nova_gem_create {
  __u32 handle;
  __u32 pad;
  __u64 size;
};
struct drm_nova_gem_info {
  __u32 handle;
  __u32 pad;
  __u64 size;
};
#define DRM_NOVA_GETPARAM 0x00
#define DRM_NOVA_GEM_CREATE 0x01
#define DRM_NOVA_GEM_INFO 0x02
enum {
  DRM_IOCTL_NOVA_GETPARAM = DRM_IOWR(DRM_COMMAND_BASE + DRM_NOVA_GETPARAM, struct drm_nova_getparam),
  DRM_IOCTL_NOVA_GEM_CREATE = DRM_IOWR(DRM_COMMAND_BASE + DRM_NOVA_GEM_CREATE, struct drm_nova_gem_create),
  DRM_IOCTL_NOVA_GEM_INFO = DRM_IOWR(DRM_COMMAND_BASE + DRM_NOVA_GEM_INFO, struct drm_nova_gem_info),
};
#ifdef __cplusplus
}
#endif
#endif
