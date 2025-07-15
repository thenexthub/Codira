//===--- SourceInfoFormat.h - Format languagesourceinfo files ---*- c++ -*---===//
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
///
/// \file Contains various constants and helper types to deal with serialized
/// source information (.codesourceinfo files).
///
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_SERIALIZATION_SOURCEINFOFORMAT_H
#define LANGUAGE_SERIALIZATION_SOURCEINFOFORMAT_H

#include "toolchain/Bitcode/BitcodeConvenience.h"

namespace language {
namespace serialization {

using toolchain::BCArray;
using toolchain::BCBlob;
using toolchain::BCFixed;
using toolchain::BCGenericRecordLayout;
using toolchain::BCRecordLayout;
using toolchain::BCVBR;

/// Magic number for serialized source info files.
const unsigned char LANGUAGESOURCEINFO_SIGNATURE[] = { 0xF0, 0x9F, 0x8F, 0x8E };

/// Serialized sourceinfo format major version number.
///
/// Increment this value when making a backwards-incompatible change, i.e. where
/// an \e old compiler will \e not be able to read the new format. This should
/// be rare. When incrementing this value, reset LANGUAGESOURCEINFO_VERSION_MINOR to 0.
///
/// See docs/StableBitcode.md for information on how to make
/// backwards-compatible changes using the LLVM bitcode format.
const uint16_t LANGUAGESOURCEINFO_VERSION_MAJOR = 3;

/// Serialized languagesourceinfo format minor version number.
///
/// Increment this value when making a backwards-compatible change that might be
/// interesting to test for. A backwards-compatible change is one where an \e
/// old compiler can read the new format without any problems (usually by
/// ignoring new information).
const uint16_t LANGUAGESOURCEINFO_VERSION_MINOR = 0; // add location offset

/// The hash seed used for the string hashes(toolchain::djbHash) in a .codesourceinfo file.
const uint32_t LANGUAGESOURCEINFO_HASH_SEED = 5387;

/// The record types within the DECL_LOCS block.
///
/// Though we strive to keep the format stable, breaking the format of
/// .codesourceinfo doesn't have consequences as serious as breaking the format
/// of .codedoc, because .codesourceinfo file is for local development use only.
///
/// When changing this block, backwards-compatible changes are preferred.
/// You may need to update the version when you do so. See docs/StableBitcode.md
/// for information on how to make backwards-compatible changes using the LLVM
/// bitcode format.
///
/// \sa DECL_LOCS_BLOCK_ID
namespace decl_locs_block {
  enum RecordKind {
    BASIC_DECL_LOCS = 1,
    DECL_USRS,
    TEXT_DATA,
    DOC_RANGES,
    SOURCE_FILE_LIST,
  };

  using SourceFileListLayout = BCRecordLayout<
    SOURCE_FILE_LIST, // record ID
    BCBlob            // An array of fixed size 'BasicSourceFileInfo' data.
  >;

  using BasicDeclLocsLayout = BCRecordLayout<
    BASIC_DECL_LOCS, // record ID
    BCBlob           // an array of fixed size location data
  >;

  using DeclUSRSLayout = BCRecordLayout<
    DECL_USRS,         // record ID
    BCVBR<16>,         // table offset within the blob (an toolchain::OnDiskHashTable)
    BCBlob             // map from actual USR to USR Id
  >;

  using TextDataLayout = BCRecordLayout<
    TEXT_DATA,        // record ID
    BCBlob            // a list of 0-terminated string segments
  >;

  using DocRangesLayout = BCRecordLayout<
    DOC_RANGES,         // record ID
    BCBlob
  >;

} // namespace sourceinfo_block

} // end namespace serialization
} // end namespace language

#endif
