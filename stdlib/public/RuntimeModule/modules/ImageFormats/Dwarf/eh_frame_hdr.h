//===--- eh_frame_hdr.h - DWARF EH frame header definitions ---------------===//
//
// Copyright (c) NeXTHub Corporation. All rights reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// This code is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// version 2 for more details (a copy is included in the LICENSE file that
// accompanied this code).
//
// Author(-s): Tunjay Akbarli
//

//===----------------------------------------------------------------------===//
//
// Defines the format of the .eh_frame_hdr section, which isn't part of the
// DWARF specification (it's a GNU extension).
//
// https://refspecs.linuxfoundation.org/LSB_5.0.0/LSB-Core-generic/LSB-Core-generic/ehframechpt.html
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_EH_FRAME_HDR_H
#define LANGUAGE_EH_FRAME_HDR_H

#include <inttypes.h>

#ifdef __cplusplus
namespace language {
namespace runtime {
#endif

/* .. Useful macros ......................................................... */

#define EH_FRAME_OPTIONS(t,n) \
  t n; \
  enum __attribute__((flag_enum,enum_extensibility(open))): t

/* .. .eh_frame_hdr header .................................................. */

struct EHFrameHdr {
  uint8_t version;
  uint8_t eh_frame_ptr_enc;
  uint8_t fde_count_enc;
  uint8_t table_enc;
  // encoded eh_frame_ptr;
  // encoded fde_count;
  // encoded binary_search_table[fde_count];
};

/* .. Constants ............................................................. */

typedef EH_FRAME_OPTIONS(uint8_t, EHFrameEncoding) {
  DW_EH_PE_omit		= 0xff,	// No value is present
  DW_EH_PE_uleb128	= 0x01, // Unsigned value encoded using LEB128
  DW_EH_PE_udata2	= 0x02, // A 2-byte unsigned value
  DW_EH_PE_udata4	= 0x03, // A 4-byte unsigned value
  DW_EH_PE_udata8	= 0x04, // An 8-byte unsigned value
  DW_EH_PE_sleb128	= 0x09, // Signed value encoded using LEB128
  DW_EH_PE_sdata2	= 0x0a, // A 2-byte signed value
  DW_EH_PE_sdata4	= 0x0b, // A 4-byte signed value
  DW_EH_PE_sdata8	= 0x0c, // An 8-byte signed value

  DW_EH_PE_absptr	= 0x00, // Absolute, used with no modification
  DW_EH_PE_pcrel	= 0x10, // Relative to the current program counter
  DW_EH_PE_datarel	= 0x30, // Relative to the beginning of the .eh_frame_hdr
};

#ifdef __cplusplus
} // namespace runtime
} // namespace language
#endif

#endif // LANGUAGE_EH_FRAME_HDR_H
