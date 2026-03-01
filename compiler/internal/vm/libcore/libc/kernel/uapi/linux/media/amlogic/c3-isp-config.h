/*
 * This file is auto-generated. Modifications will be lost.
 *
 * See https://android.googlesource.com/platform/bionic/+/master/libc/kernel/
 * for more information.
 */
#ifndef _UAPI_C3_ISP_CONFIG_H_
#define _UAPI_C3_ISP_CONFIG_H_
#include <linux/types.h>
#define C3_ISP_AE_MAX_ZONES (17 * 15)
#define C3_ISP_AF_MAX_ZONES (17 * 15)
#define C3_ISP_AWB_MAX_ZONES (32 * 24)
#define C3_ISP_AE_MAX_PT_NUM 18
#define C3_ISP_AF_MAX_PT_NUM 18
#define C3_ISP_AWB_MAX_PT_NUM 33
struct c3_isp_awb_zone_stats {
  __u16 rg;
  __u16 bg;
  __u32 pixel_sum;
};
struct c3_isp_awb_stats {
  struct c3_isp_awb_zone_stats stats[C3_ISP_AWB_MAX_ZONES];
} __attribute__((aligned(16)));
struct c3_isp_ae_zone_stats {
  __u16 hist0;
  __u16 hist1;
  __u16 hist3;
  __u16 hist4;
};
struct c3_isp_ae_stats {
  struct c3_isp_ae_zone_stats stats[C3_ISP_AE_MAX_ZONES];
  __u32 reserved[2];
  __u32 hist[1024];
} __attribute__((aligned(16)));
struct c3_isp_af_zone_stats {
  __u16 i2_mat;
  __u16 i4_mat;
  __u16 e4_mat;
  __u16 e4_exp : 5;
  __u16 i2_exp : 5;
  __u16 i4_exp : 6;
};
struct c3_isp_af_stats {
  struct c3_isp_af_zone_stats stats[C3_ISP_AF_MAX_ZONES];
  __u32 reserved[2];
} __attribute__((aligned(16)));
struct c3_isp_stats_info {
  struct c3_isp_awb_stats awb;
  struct c3_isp_ae_stats ae;
  struct c3_isp_af_stats af;
};
enum c3_isp_params_buffer_version {
  C3_ISP_PARAMS_BUFFER_V0,
};
enum c3_isp_params_block_type {
  C3_ISP_PARAMS_BLOCK_AWB_GAINS,
  C3_ISP_PARAMS_BLOCK_AWB_CONFIG,
  C3_ISP_PARAMS_BLOCK_AE_CONFIG,
  C3_ISP_PARAMS_BLOCK_AF_CONFIG,
  C3_ISP_PARAMS_BLOCK_PST_GAMMA,
  C3_ISP_PARAMS_BLOCK_CCM,
  C3_ISP_PARAMS_BLOCK_CSC,
  C3_ISP_PARAMS_BLOCK_BLC,
  C3_ISP_PARAMS_BLOCK_SENTINEL
};
#define C3_ISP_PARAMS_BLOCK_FL_DISABLE (1U << 0)
#define C3_ISP_PARAMS_BLOCK_FL_ENABLE (1U << 1)
struct c3_isp_params_block_header {
  __u16 type;
  __u16 flags;
  __u32 size;
};
struct c3_isp_params_awb_gains {
  struct c3_isp_params_block_header header;
  __u16 gr_gain;
  __u16 r_gain;
  __u16 b_gain;
  __u16 gb_gain;
} __attribute__((aligned(8)));
enum c3_isp_params_awb_tap_points {
  C3_ISP_AWB_STATS_TAP_OFE = 0,
  C3_ISP_AWB_STATS_TAP_GE,
  C3_ISP_AWB_STATS_TAP_BEFORE_WB,
  C3_ISP_AWB_STATS_TAP_AFTER_WB,
};
struct c3_isp_params_awb_config {
  struct c3_isp_params_block_header header;
  __u8 tap_point;
  __u8 satur_vald;
  __u8 horiz_zones_num;
  __u8 vert_zones_num;
  __u16 rg_min;
  __u16 rg_max;
  __u16 bg_min;
  __u16 bg_max;
  __u16 rg_low;
  __u16 rg_high;
  __u16 bg_low;
  __u16 bg_high;
  __u8 zone_weight[C3_ISP_AWB_MAX_ZONES];
  __u16 horiz_coord[C3_ISP_AWB_MAX_PT_NUM];
  __u16 vert_coord[C3_ISP_AWB_MAX_PT_NUM];
} __attribute__((aligned(8)));
enum c3_isp_params_ae_tap_points {
  C3_ISP_AE_STATS_TAP_GE = 0,
  C3_ISP_AE_STATS_TAP_MLS,
};
struct c3_isp_params_ae_config {
  struct c3_isp_params_block_header header;
  __u8 tap_point;
  __u8 horiz_zones_num;
  __u8 vert_zones_num;
  __u8 zone_weight[C3_ISP_AE_MAX_ZONES];
  __u16 horiz_coord[C3_ISP_AE_MAX_PT_NUM];
  __u16 vert_coord[C3_ISP_AE_MAX_PT_NUM];
  __u16 reserved[3];
} __attribute__((aligned(8)));
enum c3_isp_params_af_tap_points {
  C3_ISP_AF_STATS_TAP_SNR = 0,
  C3_ISP_AF_STATS_TAP_DMS,
};
struct c3_isp_params_af_config {
  struct c3_isp_params_block_header header;
  __u8 tap_point;
  __u8 horiz_zones_num;
  __u8 vert_zones_num;
  __u8 reserved[5];
  __u16 horiz_coord[C3_ISP_AF_MAX_PT_NUM];
  __u16 vert_coord[C3_ISP_AF_MAX_PT_NUM];
} __attribute__((aligned(8)));
struct c3_isp_params_pst_gamma {
  struct c3_isp_params_block_header header;
  __u16 lut[129];
  __u16 reserved[3];
} __attribute__((aligned(8)));
struct c3_isp_params_ccm {
  struct c3_isp_params_block_header header;
  __s16 matrix[3][3];
  __u16 reserved[3];
} __attribute__((aligned(8)));
struct c3_isp_params_csc {
  struct c3_isp_params_block_header header;
  __s16 matrix[3][3];
  __u16 reserved[3];
} __attribute__((aligned(8)));
struct c3_isp_params_blc {
  struct c3_isp_params_block_header header;
  __u16 gr_ofst;
  __u16 r_ofst;
  __u16 b_ofst;
  __u16 gb_ofst;
};
#define C3_ISP_PARAMS_MAX_SIZE (sizeof(struct c3_isp_params_awb_gains) + sizeof(struct c3_isp_params_awb_config) + sizeof(struct c3_isp_params_ae_config) + sizeof(struct c3_isp_params_af_config) + sizeof(struct c3_isp_params_pst_gamma) + sizeof(struct c3_isp_params_ccm) + sizeof(struct c3_isp_params_csc) + sizeof(struct c3_isp_params_blc))
struct c3_isp_params_cfg {
  __u32 version;
  __u32 data_size;
  __u8 data[C3_ISP_PARAMS_MAX_SIZE];
};
#endif
