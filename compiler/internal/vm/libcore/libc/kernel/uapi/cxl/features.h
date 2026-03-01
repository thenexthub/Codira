/*
 * This file is auto-generated. Modifications will be lost.
 *
 * See https://android.googlesource.com/platform/bionic/+/master/libc/kernel/
 * for more information.
 */
#ifndef _UAPI_CXL_FEATURES_H_
#define _UAPI_CXL_FEATURES_H_
#include <linux/types.h>
typedef unsigned char __uapi_uuid_t[16];
struct cxl_mbox_get_sup_feats_in {
  __le32 count;
  __le16 start_idx;
  __u8 reserved[2];
} __attribute__((__packed__));
#define CXL_CMD_CONFIG_CHANGE_COLD_RESET BIT(0)
#define CXL_CMD_CONFIG_CHANGE_IMMEDIATE BIT(1)
#define CXL_CMD_DATA_CHANGE_IMMEDIATE BIT(2)
#define CXL_CMD_POLICY_CHANGE_IMMEDIATE BIT(3)
#define CXL_CMD_LOG_CHANGE_IMMEDIATE BIT(4)
#define CXL_CMD_SECURITY_STATE_CHANGE BIT(5)
#define CXL_CMD_BACKGROUND BIT(6)
#define CXL_CMD_BGCMD_ABORT_SUPPORTED BIT(7)
#define CXL_CMD_EFFECTS_VALID BIT(9)
#define CXL_CMD_CONFIG_CHANGE_CONV_RESET BIT(10)
#define CXL_CMD_CONFIG_CHANGE_CXL_RESET BIT(11)
#define CXL_CMD_EFFECTS_RESERVED GENMASK(15, 12)
struct cxl_feat_entry {
  __uapi_uuid_t uuid;
  __le16 id;
  __le16 get_feat_size;
  __le16 set_feat_size;
  __le32 flags;
  __u8 get_feat_ver;
  __u8 set_feat_ver;
  __le16 effects;
  __u8 reserved[18];
} __attribute__((__packed__));
#define CXL_FEATURE_F_CHANGEABLE BIT(0)
#define CXL_FEATURE_F_PERSIST_FW_UPDATE BIT(4)
#define CXL_FEATURE_F_DEFAULT_SEL BIT(5)
#define CXL_FEATURE_F_SAVED_SEL BIT(6)
struct cxl_mbox_get_sup_feats_out {
  __struct_group(cxl_mbox_get_sup_feats_out_hdr, hdr,, __le16 num_entries;
  __le16 supported_feats;
  __u8 reserved[4];
 );
  struct cxl_feat_entry ents[] __counted_by_le(num_entries);
} __attribute__((__packed__));
struct cxl_mbox_get_feat_in {
  __uapi_uuid_t uuid;
  __le16 offset;
  __le16 count;
  __u8 selection;
} __attribute__((__packed__));
enum cxl_get_feat_selection {
  CXL_GET_FEAT_SEL_CURRENT_VALUE,
  CXL_GET_FEAT_SEL_DEFAULT_VALUE,
  CXL_GET_FEAT_SEL_SAVED_VALUE,
  CXL_GET_FEAT_SEL_MAX
};
struct cxl_mbox_set_feat_in {
  __struct_group(cxl_mbox_set_feat_hdr, hdr,, __uapi_uuid_t uuid;
  __le32 flags;
  __le16 offset;
  __u8 version;
  __u8 rsvd[9];
 );
  __u8 feat_data[];
} __attribute__((__packed__));
enum cxl_set_feat_flag_data_transfer {
  CXL_SET_FEAT_FLAG_FULL_DATA_TRANSFER = 0,
  CXL_SET_FEAT_FLAG_INITIATE_DATA_TRANSFER,
  CXL_SET_FEAT_FLAG_CONTINUE_DATA_TRANSFER,
  CXL_SET_FEAT_FLAG_FINISH_DATA_TRANSFER,
  CXL_SET_FEAT_FLAG_ABORT_DATA_TRANSFER,
  CXL_SET_FEAT_FLAG_DATA_TRANSFER_MAX
};
#define CXL_SET_FEAT_FLAG_DATA_TRANSFER_MASK GENMASK(2, 0)
#define CXL_SET_FEAT_FLAG_DATA_SAVED_ACROSS_RESET BIT(3)
#endif
