//===--- DocFormat.h - The internals of languagedoc files ----------*- C++ -*-===//
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
/// documentation info (languagedoc files).
///
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_SERIALIZATION_DOCFORMAT_H
#define LANGUAGE_SERIALIZATION_DOCFORMAT_H

#include "toolchain/Bitcode/BitcodeConvenience.h"

namespace language {
namespace serialization {

using toolchain::BCArray;
using toolchain::BCBlob;
using toolchain::BCFixed;
using toolchain::BCGenericRecordLayout;
using toolchain::BCRecordLayout;
using toolchain::BCVBR;

/// Magic number for serialized documentation files.
const unsigned char LANGUAGEDOC_SIGNATURE[] = { 0xE2, 0x9C, 0xA8, 0x07 };

/// Serialized languagedoc format major version number.
///
/// Increment this value when making a backwards-incompatible change, i.e. where
/// an \e old compiler will \e not be able to read the new format. This should
/// be rare. When incrementing this value, reset LANGUAGEDOC_VERSION_MINOR to 0.
///
/// See docs/StableBitcode.md for information on how to make
/// backwards-compatible changes using the LLVM bitcode format.
const uint16_t LANGUAGEDOC_VERSION_MAJOR = 1;

/// Serialized languagedoc format minor version number.
///
/// Increment this value when making a backwards-compatible change that might be
/// interesting to test for. A backwards-compatible change is one where an \e
/// old compiler can read the new format without any problems (usually by
/// ignoring new information).
///
/// If the \e new compiler can treat the new and old format identically, or if
/// the presence of a new record, block, or field is sufficient to indicate that
/// the languagedoc file is using a new format, it is okay not to increment this
/// value. However, it may be interesting for a new compiler to treat the \e
/// absence of information differently for the old and new formats; in this
/// case, the difference in minor version number can distinguish the two.
///
/// The minor version number does not need to be changed simply to track which
/// compiler generated a languagedoc file; the full compiler version is already
/// stored as text and can be checked by running the \c strings command-line
/// tool on a languagedoc file.
///
/// To ensure that two separate changes don't silently get merged into one in
/// source control, you should also update the comment to briefly describe what
/// change you made. The content of this comment isn't important; it just
/// ensures a conflict if two people change the module format. Don't worry about
/// adhering to the 80-column limit for this line.
const uint16_t LANGUAGEDOC_VERSION_MINOR = 1; // Last change: skipping 0 for testing purposes

/// The hash seed used for the string hashes in a Codira 5.1 languagedoc file.
///
/// 0 is not a good seed for toolchain::djbHash, but languagedoc files use a stable
/// format, so we can't change the hash seed without a version bump. Any new
/// hashed strings should use a new stable hash seed constant. (No such constant
/// has been defined at the time this doc comment was last updated because there
/// are no other strings to hash.)
const uint32_t LANGUAGEDOC_HASH_SEED_5_1 = 0;

/// The record types within the comment block.
///
/// Be very careful when changing this block; it must remain
/// backwards-compatible. Adding new records is okay---they will be ignored---
/// but modifying existing ones must be done carefully. You may need to update
/// the version when you do so. See docs/StableBitcode.md for information on how
/// to make backwards-compatible changes using the LLVM bitcode format.
///
/// \sa COMMENT_BLOCK_ID
namespace comment_block {
  enum RecordKind {
    DECL_COMMENTS = 1,
    GROUP_NAMES = 2,
  };

  using DeclCommentListLayout = BCRecordLayout<
    DECL_COMMENTS, // record ID
    BCVBR<16>,     // table offset within the blob (an toolchain::OnDiskHashTable)
    BCBlob         // map from Decl USRs to comments
  >;

  using GroupNamesLayout = BCRecordLayout<
    GROUP_NAMES,    // record ID
    BCBlob          // actual names
  >;

} // namespace comment_block

} // end namespace serialization
} // end namespace language

#endif
