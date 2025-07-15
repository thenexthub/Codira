//===--- ImageInspectionCommon.h - Image inspection routines -----*- C++ -*-===//
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
/// \file
///
/// This file unifies common ELF and COFF image inspection routines.
///
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_RUNTIME_IMAGEINSPECTIONCOMMON_H
#define LANGUAGE_RUNTIME_IMAGEINSPECTIONCOMMON_H

#if defined(__MACH__)

/// The Mach-O section name for the section containing protocol descriptor
/// references. This lives within SEG_TEXT.
#define MachOProtocolsSection "__language5_protos"
/// The Mach-O section name for the section containing protocol conformances.
/// This lives within SEG_TEXT.
#define MachOProtocolConformancesSection "__language5_proto"
/// The Mach-O section name for the section containing copyable type references.
/// This lives within SEG_TEXT.
#define MachOTypeMetadataRecordSection "__language5_types"
/// The Mach-O section name for the section containing additional type references.
/// This lives within SEG_TEXT.
#define MachOExtraTypeMetadataRecordSection "__language5_types2"
/// The Mach-O section name for the section containing dynamic replacements.
/// This lives within SEG_TEXT.
#define MachODynamicReplacementSection "__language5_replace"
#define MachODynamicReplacementSomeSection "__language5_replac2"
/// The Mach-O section name for the section containing accessible functions.
/// This lives within SEG_TEXT.
#define MachOAccessibleFunctionsSection "__language5_acfuncs"

#define MachOTextSegment "__TEXT"

#else

#include "language/shims/Visibility.h"
#include <cstdint>
#include <cstddef>

namespace language {
struct MetadataSections;
static constexpr const uintptr_t CurrentSectionMetadataVersion = 4;
}

struct SectionInfo {
  uint64_t size;
  const char *data;
};

/// Called by injected constructors when a dynamic library is loaded.
///
/// \param sections A structure describing the metadata sections in the
///     newly-loaded image.
///
/// \warning The runtime keeps a reference to \a sections and may mutate it, so
///   it \em must be mutable and long-lived (that is, statically or dynamically
///   allocated.) The effect of passing a pointer to a local value is undefined.
LANGUAGE_RUNTIME_EXPORT
void language_addNewDSOImage(struct language::MetadataSections *sections);

/// Enumerate all metadata sections in the current process that are known to the
/// Codira runtime.
///
/// \param body A function to invoke once per metadata sections structure.
///   If this function returns \c false, enumeration is stopped.
/// \param context An additional context pointer to pass to \a body.
///
/// On Mach-O-based platforms (i.e. Apple platforms), this function is
/// unavailable. On those platforms, use dyld API to enumerate loaded images and
/// their corresponding metadata sections.
LANGUAGE_RUNTIME_EXPORT LANGUAGE_WEAK_IMPORT
void language_enumerateAllMetadataSections(
  bool (* body)(const language::MetadataSections *sections, void *context),
  void *context
);

#ifndef NDEBUG

LANGUAGE_RUNTIME_EXPORT
const char *
language_getMetadataSectionName(const struct language::MetadataSections *section);

LANGUAGE_RUNTIME_EXPORT
void language_getMetadataSectionBaseAddress(
  const struct language::MetadataSections *section,
  void const **out_actual, void const **out_expected);

LANGUAGE_RUNTIME_EXPORT
size_t language_getMetadataSectionCount();

#endif // NDEBUG

#endif // !defined(__MACH__)

#endif // LANGUAGE_RUNTIME_IMAGEINSPECTIONCOMMON_H
