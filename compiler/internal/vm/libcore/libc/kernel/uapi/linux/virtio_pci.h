/*
 * This file is auto-generated. Modifications will be lost.
 *
 * See https://android.googlesource.com/platform/bionic/+/master/libc/kernel/
 * for more information.
 */
#ifndef _LINUX_VIRTIO_PCI_H
#define _LINUX_VIRTIO_PCI_H
#include <linux/types.h>
#include <linux/kernel.h>
#ifndef VIRTIO_PCI_NO_LEGACY
#define VIRTIO_PCI_HOST_FEATURES 0
#define VIRTIO_PCI_GUEST_FEATURES 4
#define VIRTIO_PCI_QUEUE_PFN 8
#define VIRTIO_PCI_QUEUE_NUM 12
#define VIRTIO_PCI_QUEUE_SEL 14
#define VIRTIO_PCI_QUEUE_NOTIFY 16
#define VIRTIO_PCI_STATUS 18
#define VIRTIO_PCI_ISR 19
#define VIRTIO_MSI_CONFIG_VECTOR 20
#define VIRTIO_MSI_QUEUE_VECTOR 22
#define VIRTIO_PCI_CONFIG_OFF(msix_enabled) ((msix_enabled) ? 24 : 20)
#define VIRTIO_PCI_CONFIG(dev) VIRTIO_PCI_CONFIG_OFF((dev)->msix_enabled)
#define VIRTIO_PCI_ABI_VERSION 0
#define VIRTIO_PCI_QUEUE_ADDR_SHIFT 12
#define VIRTIO_PCI_VRING_ALIGN 4096
#endif
#define VIRTIO_PCI_ISR_CONFIG 0x2
#define VIRTIO_MSI_NO_VECTOR 0xffff
#ifndef VIRTIO_PCI_NO_MODERN
#define VIRTIO_PCI_CAP_COMMON_CFG 1
#define VIRTIO_PCI_CAP_NOTIFY_CFG 2
#define VIRTIO_PCI_CAP_ISR_CFG 3
#define VIRTIO_PCI_CAP_DEVICE_CFG 4
#define VIRTIO_PCI_CAP_PCI_CFG 5
#define VIRTIO_PCI_CAP_SHARED_MEMORY_CFG 8
#define VIRTIO_PCI_CAP_VENDOR_CFG 9
struct virtio_pci_cap {
  __u8 cap_vndr;
  __u8 cap_next;
  __u8 cap_len;
  __u8 cfg_type;
  __u8 bar;
  __u8 id;
  __u8 padding[2];
  __le32 offset;
  __le32 length;
};
struct virtio_pci_vndr_data {
  __u8 cap_vndr;
  __u8 cap_next;
  __u8 cap_len;
  __u8 cfg_type;
  __u16 vendor_id;
};
struct virtio_pci_cap64 {
  struct virtio_pci_cap cap;
  __le32 offset_hi;
  __le32 length_hi;
};
struct virtio_pci_notify_cap {
  struct virtio_pci_cap cap;
  __le32 notify_off_multiplier;
};
struct virtio_pci_common_cfg {
  __le32 device_feature_select;
  __le32 device_feature;
  __le32 guest_feature_select;
  __le32 guest_feature;
  __le16 msix_config;
  __le16 num_queues;
  __u8 device_status;
  __u8 config_generation;
  __le16 queue_select;
  __le16 queue_size;
  __le16 queue_msix_vector;
  __le16 queue_enable;
  __le16 queue_notify_off;
  __le32 queue_desc_lo;
  __le32 queue_desc_hi;
  __le32 queue_avail_lo;
  __le32 queue_avail_hi;
  __le32 queue_used_lo;
  __le32 queue_used_hi;
};
struct virtio_pci_modern_common_cfg {
  struct virtio_pci_common_cfg cfg;
  __le16 queue_notify_data;
  __le16 queue_reset;
  __le16 admin_queue_index;
  __le16 admin_queue_num;
};
struct virtio_pci_cfg_cap {
  struct virtio_pci_cap cap;
  __u8 pci_cfg_data[4];
};
#define VIRTIO_PCI_CAP_VNDR 0
#define VIRTIO_PCI_CAP_NEXT 1
#define VIRTIO_PCI_CAP_LEN 2
#define VIRTIO_PCI_CAP_CFG_TYPE 3
#define VIRTIO_PCI_CAP_BAR 4
#define VIRTIO_PCI_CAP_OFFSET 8
#define VIRTIO_PCI_CAP_LENGTH 12
#define VIRTIO_PCI_NOTIFY_CAP_MULT 16
#define VIRTIO_PCI_COMMON_DFSELECT 0
#define VIRTIO_PCI_COMMON_DF 4
#define VIRTIO_PCI_COMMON_GFSELECT 8
#define VIRTIO_PCI_COMMON_GF 12
#define VIRTIO_PCI_COMMON_MSIX 16
#define VIRTIO_PCI_COMMON_NUMQ 18
#define VIRTIO_PCI_COMMON_STATUS 20
#define VIRTIO_PCI_COMMON_CFGGENERATION 21
#define VIRTIO_PCI_COMMON_Q_SELECT 22
#define VIRTIO_PCI_COMMON_Q_SIZE 24
#define VIRTIO_PCI_COMMON_Q_MSIX 26
#define VIRTIO_PCI_COMMON_Q_ENABLE 28
#define VIRTIO_PCI_COMMON_Q_NOFF 30
#define VIRTIO_PCI_COMMON_Q_DESCLO 32
#define VIRTIO_PCI_COMMON_Q_DESCHI 36
#define VIRTIO_PCI_COMMON_Q_AVAILLO 40
#define VIRTIO_PCI_COMMON_Q_AVAILHI 44
#define VIRTIO_PCI_COMMON_Q_USEDLO 48
#define VIRTIO_PCI_COMMON_Q_USEDHI 52
#define VIRTIO_PCI_COMMON_Q_NDATA 56
#define VIRTIO_PCI_COMMON_Q_RESET 58
#define VIRTIO_PCI_COMMON_ADM_Q_IDX 60
#define VIRTIO_PCI_COMMON_ADM_Q_NUM 62
#endif
#define VIRTIO_ADMIN_STATUS_OK 0
#define VIRTIO_ADMIN_CMD_LIST_QUERY 0x0
#define VIRTIO_ADMIN_CMD_LIST_USE 0x1
#define VIRTIO_ADMIN_GROUP_TYPE_SELF 0x0
#define VIRTIO_ADMIN_GROUP_TYPE_SRIOV 0x1
#define VIRTIO_ADMIN_CMD_LEGACY_COMMON_CFG_WRITE 0x2
#define VIRTIO_ADMIN_CMD_LEGACY_COMMON_CFG_READ 0x3
#define VIRTIO_ADMIN_CMD_LEGACY_DEV_CFG_WRITE 0x4
#define VIRTIO_ADMIN_CMD_LEGACY_DEV_CFG_READ 0x5
#define VIRTIO_ADMIN_CMD_LEGACY_NOTIFY_INFO 0x6
#define VIRTIO_ADMIN_CMD_CAP_ID_LIST_QUERY 0x7
#define VIRTIO_ADMIN_CMD_DEVICE_CAP_GET 0x8
#define VIRTIO_ADMIN_CMD_DRIVER_CAP_SET 0x9
#define VIRTIO_ADMIN_CMD_RESOURCE_OBJ_CREATE 0xa
#define VIRTIO_ADMIN_CMD_RESOURCE_OBJ_DESTROY 0xd
#define VIRTIO_ADMIN_CMD_DEV_PARTS_METADATA_GET 0xe
#define VIRTIO_ADMIN_CMD_DEV_PARTS_GET 0xf
#define VIRTIO_ADMIN_CMD_DEV_PARTS_SET 0x10
#define VIRTIO_ADMIN_CMD_DEV_MODE_SET 0x11
struct virtio_admin_cmd_hdr {
  __le16 opcode;
  __le16 group_type;
  __u8 reserved1[12];
  __le64 group_member_id;
};
struct virtio_admin_cmd_status {
  __le16 status;
  __le16 status_qualifier;
  __u8 reserved2[4];
};
struct virtio_admin_cmd_legacy_wr_data {
  __u8 offset;
  __u8 reserved[7];
  __u8 registers[];
};
struct virtio_admin_cmd_legacy_rd_data {
  __u8 offset;
};
#define VIRTIO_ADMIN_CMD_NOTIFY_INFO_FLAGS_END 0
#define VIRTIO_ADMIN_CMD_NOTIFY_INFO_FLAGS_OWNER_DEV 0x1
#define VIRTIO_ADMIN_CMD_NOTIFY_INFO_FLAGS_OWNER_MEM 0x2
#define VIRTIO_ADMIN_CMD_MAX_NOTIFY_INFO 4
struct virtio_admin_cmd_notify_info_data {
  __u8 flags;
  __u8 bar;
  __u8 padding[6];
  __le64 offset;
};
struct virtio_admin_cmd_notify_info_result {
  struct virtio_admin_cmd_notify_info_data entries[VIRTIO_ADMIN_CMD_MAX_NOTIFY_INFO];
};
#define VIRTIO_DEV_PARTS_CAP 0x0000
struct virtio_dev_parts_cap {
  __u8 get_parts_resource_objects_limit;
  __u8 set_parts_resource_objects_limit;
};
#define MAX_CAP_ID __KERNEL_DIV_ROUND_UP(VIRTIO_DEV_PARTS_CAP + 1, 64)
struct virtio_admin_cmd_query_cap_id_result {
  __le64 supported_caps[MAX_CAP_ID];
};
struct virtio_admin_cmd_cap_get_data {
  __le16 id;
  __u8 reserved[6];
};
struct virtio_admin_cmd_cap_set_data {
  __le16 id;
  __u8 reserved[6];
  __u8 cap_specific_data[];
};
struct virtio_admin_cmd_resource_obj_cmd_hdr {
  __le16 type;
  __u8 reserved[2];
  __le32 id;
};
struct virtio_admin_cmd_resource_obj_create_data {
  struct virtio_admin_cmd_resource_obj_cmd_hdr hdr;
  __le64 flags;
  __u8 resource_obj_specific_data[];
};
#define VIRTIO_RESOURCE_OBJ_DEV_PARTS 0
#define VIRTIO_RESOURCE_OBJ_DEV_PARTS_TYPE_GET 0
#define VIRTIO_RESOURCE_OBJ_DEV_PARTS_TYPE_SET 1
struct virtio_resource_obj_dev_parts {
  __u8 type;
  __u8 reserved[7];
};
#define VIRTIO_ADMIN_CMD_DEV_PARTS_METADATA_TYPE_SIZE 0
#define VIRTIO_ADMIN_CMD_DEV_PARTS_METADATA_TYPE_COUNT 1
#define VIRTIO_ADMIN_CMD_DEV_PARTS_METADATA_TYPE_LIST 2
struct virtio_admin_cmd_dev_parts_metadata_data {
  struct virtio_admin_cmd_resource_obj_cmd_hdr hdr;
  __u8 type;
  __u8 reserved[7];
};
#define VIRTIO_DEV_PART_F_OPTIONAL 0
struct virtio_dev_part_hdr {
  __le16 part_type;
  __u8 flags;
  __u8 reserved;
  union {
    struct {
      __le32 offset;
      __le32 reserved;
    } pci_common_cfg;
    struct {
      __le16 index;
      __u8 reserved[6];
    } vq_index;
  } selector;
  __le32 length;
};
struct virtio_dev_part {
  struct virtio_dev_part_hdr hdr;
  __u8 value[];
};
struct virtio_admin_cmd_dev_parts_metadata_result {
  union {
    struct {
      __le32 size;
      __le32 reserved;
    } parts_size;
    struct {
      __le32 count;
      __le32 reserved;
    } hdr_list_count;
    struct {
      __le32 count;
      __le32 reserved;
      struct virtio_dev_part_hdr hdrs[];
    } hdr_list;
  };
};
#define VIRTIO_ADMIN_CMD_DEV_PARTS_GET_TYPE_SELECTED 0
#define VIRTIO_ADMIN_CMD_DEV_PARTS_GET_TYPE_ALL 1
struct virtio_admin_cmd_dev_parts_get_data {
  struct virtio_admin_cmd_resource_obj_cmd_hdr hdr;
  __u8 type;
  __u8 reserved[7];
  struct virtio_dev_part_hdr hdr_list[];
};
struct virtio_admin_cmd_dev_parts_set_data {
  struct virtio_admin_cmd_resource_obj_cmd_hdr hdr;
  struct virtio_dev_part parts[];
};
#define VIRTIO_ADMIN_CMD_DEV_MODE_F_STOPPED 0
struct virtio_admin_cmd_dev_mode_set_data {
  __u8 flags;
};
#endif
