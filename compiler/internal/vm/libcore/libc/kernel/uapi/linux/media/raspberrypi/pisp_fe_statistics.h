/*
 * This file is auto-generated. Modifications will be lost.
 *
 * See https://android.googlesource.com/platform/bionic/+/master/libc/kernel/
 * for more information.
 */
#ifndef _UAPI_PISP_FE_STATISTICS_H_
#define _UAPI_PISP_FE_STATISTICS_H_
#include <linux/types.h>
#define PISP_FLOATING_STATS_NUM_ZONES 4
#define PISP_AGC_STATS_NUM_BINS 1024
#define PISP_AGC_STATS_SIZE 16
#define PISP_AGC_STATS_NUM_ZONES (PISP_AGC_STATS_SIZE * PISP_AGC_STATS_SIZE)
#define PISP_AGC_STATS_NUM_ROW_SUMS 512
struct pisp_agc_statistics_zone {
  __u64 Y_sum;
  __u32 counted;
  __u32 pad;
} __attribute__((packed));
struct pisp_agc_statistics {
  __u32 row_sums[PISP_AGC_STATS_NUM_ROW_SUMS];
  __u32 histogram[PISP_AGC_STATS_NUM_BINS];
  struct pisp_agc_statistics_zone floating[PISP_FLOATING_STATS_NUM_ZONES];
} __attribute__((packed));
#define PISP_AWB_STATS_SIZE 32
#define PISP_AWB_STATS_NUM_ZONES (PISP_AWB_STATS_SIZE * PISP_AWB_STATS_SIZE)
struct pisp_awb_statistics_zone {
  __u32 R_sum;
  __u32 G_sum;
  __u32 B_sum;
  __u32 counted;
} __attribute__((packed));
struct pisp_awb_statistics {
  struct pisp_awb_statistics_zone zones[PISP_AWB_STATS_NUM_ZONES];
  struct pisp_awb_statistics_zone floating[PISP_FLOATING_STATS_NUM_ZONES];
} __attribute__((packed));
#define PISP_CDAF_STATS_SIZE 8
#define PISP_CDAF_STATS_NUM_FOMS (PISP_CDAF_STATS_SIZE * PISP_CDAF_STATS_SIZE)
struct pisp_cdaf_statistics {
  __u64 foms[PISP_CDAF_STATS_NUM_FOMS];
  __u64 floating[PISP_FLOATING_STATS_NUM_ZONES];
} __attribute__((packed));
struct pisp_statistics {
  struct pisp_awb_statistics awb;
  struct pisp_agc_statistics agc;
  struct pisp_cdaf_statistics cdaf;
} __attribute__((packed));
#endif
