/*
 * This file is auto-generated. Modifications will be lost.
 *
 * See https://android.googlesource.com/platform/bionic/+/master/libc/kernel/
 * for more information.
 */
#ifndef _UAPI_LINUX_BITS_H
#define _UAPI_LINUX_BITS_H
#define __GENMASK(h,l) (((~_UL(0)) << (l)) & (~_UL(0) >> (__BITS_PER_LONG - 1 - (h))))
#define __GENMASK_ULL(h,l) (((~_ULL(0)) << (l)) & (~_ULL(0) >> (__BITS_PER_LONG_LONG - 1 - (h))))
#define __GENMASK_U128(h,l) ((_BIT128((h)) << 1) - (_BIT128(l)))
#endif
