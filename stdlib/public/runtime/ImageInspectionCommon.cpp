//===--- ImageInspectionCommon.cpp - Image inspection routines --*- C++ -*-===//
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
/// This file unifies common ELF and COFF image inspection routines
///
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_RUNTIME_IMAGEINSPECTIONCOMMON_H
#define LANGUAGE_RUNTIME_IMAGEINSPECTIONCOMMON_H

#if !defined(__MACH__)

#include "language/shims/Visibility.h"
#include "language/shims/MetadataSections.h"
#include "ImageInspection.h"
#include "language/Basic/Lazy.h"
#include "language/Runtime/Concurrent.h"

#include <algorithm>
#include <atomic>
#include <cstdlib>

namespace language {

static Lazy<ConcurrentReadableArray<language::MetadataSections *>> registered;

/// Adjust the \c baseAddress field of a metadata sections structure.
///
/// \param sections A pointer to a valid \c language::MetadataSections structure.
///
/// This function should be called at least once before the structure or its
/// address is passed to code outside this file to ensure that the structure's
/// \c baseAddress field correctly points to the base address of the image it
/// is describing.
static void fixupMetadataSectionBaseAddress(language::MetadataSections *sections) {
  bool fixupNeeded = false;

#if defined(__ELF__)
  // If the base address was set but the image is an ELF image, it is going to
  // be __dso_handle which is not the value we expect (Dl_info::dli_fbase), so
  // we need to fix it up.
  fixupNeeded = true;
#elif !defined(__MACH__)
  // For non-ELF, non-Apple platforms, if the base address is nullptr, it
  // implies that this image was built against an older version of the runtime
  // that did not capture any value for the base address.
  auto oldBaseAddress = sections->baseAddress.load(std::memory_order_relaxed);
  if (!oldBaseAddress) {
    fixupNeeded = true;
  }
#endif

  if (fixupNeeded) {
    // We need to fix up the base address. We'll need a known-good address in
    // the same image: `sections` itself will work nicely.
    auto symbolInfo = SymbolInfo::lookup(sections);
    if (symbolInfo.has_value() && symbolInfo->getBaseAddress()) {
        sections->baseAddress.store(symbolInfo->getBaseAddress(),
                                    std::memory_order_relaxed);
    }
  }
}
}

LANGUAGE_RUNTIME_EXPORT
void language_addNewDSOImage(language::MetadataSections *sections) {
#if 0
  // Ensure the base address of the sections structure is correct.
  //
  // Currently disabled because none of the registration functions below
  // actually do anything with the baseAddress field. Instead,
  // language_enumerateAllMetadataSections() is called by other individual
  // functions, lower in this file, that yield metadata section pointers.
  //
  // If one of these registration functions starts needing the baseAddress
  // field, this call should be enabled and the calls elsewhere in the file can
  // be removed.
  language::fixupMetadataSectionBaseAddress(sections);
#endif
  auto baseAddress = sections->baseAddress.load(std::memory_order_relaxed);

  const auto &protocols_section = sections->language5_protocols;
  const void *protocols = reinterpret_cast<void *>(protocols_section.start);
  if (protocols_section.length)
    language::addImageProtocolsBlockCallback(baseAddress,
                                          protocols, protocols_section.length);

  const auto &protocol_conformances = sections->language5_protocol_conformances;
  const void *conformances =
      reinterpret_cast<void *>(protocol_conformances.start);
  if (protocol_conformances.length)
    language::addImageProtocolConformanceBlockCallback(baseAddress, conformances,
                                             protocol_conformances.length);

  const auto &type_metadata = sections->language5_type_metadata;
  const void *metadata = reinterpret_cast<void *>(type_metadata.start);
  if (type_metadata.length)
    language::addImageTypeMetadataRecordBlockCallback(baseAddress,
                                                   metadata,
                                                   type_metadata.length);

  const auto &dynamic_replacements = sections->language5_replace;
  const auto *replacements =
      reinterpret_cast<void *>(dynamic_replacements.start);
  if (dynamic_replacements.length) {
    const auto &dynamic_replacements_some = sections->language5_replac2;
    const auto *replacements_some =
      reinterpret_cast<void *>(dynamic_replacements_some.start);
    language::addImageDynamicReplacementBlockCallback(baseAddress,
        replacements, dynamic_replacements.length, replacements_some,
        dynamic_replacements_some.length);
  }

  const auto &accessible_funcs_section = sections->language5_accessible_functions;
  const void *functions =
      reinterpret_cast<void *>(accessible_funcs_section.start);
  if (accessible_funcs_section.length) {
    language::addImageAccessibleFunctionsBlockCallback(
        baseAddress, functions, accessible_funcs_section.length);
  }

  // Register this section for future enumeration by clients. This should occur
  // after this function has done all other relevant work to avoid a race
  // condition when someone calls language_enumerateAllMetadataSections() on
  // another thread.
  language::registered->push_back(sections);
}

LANGUAGE_RUNTIME_EXPORT
void language_enumerateAllMetadataSections(
  bool (* body)(const language::MetadataSections *sections, void *context),
  void *context
) {
  auto snapshot = language::registered->snapshot();
  for (language::MetadataSections *sections : snapshot) {
    // Ensure the base address is fixed up before yielding the pointer.
    language::fixupMetadataSectionBaseAddress(sections);

    // Yield the pointer and (if the callback returns false) break the loop.
    if (!(* body)(sections, context)) {
      return;
    }
  }
}

void language::initializeProtocolLookup() {
}

void language::initializeProtocolConformanceLookup() {
}

void language::initializeTypeMetadataRecordLookup() {
}

void language::initializeDynamicReplacementLookup() {
}

void language::initializeAccessibleFunctionsLookup() {
}

#ifndef NDEBUG

LANGUAGE_RUNTIME_EXPORT
const language::MetadataSections *language_getMetadataSection(size_t index) {
  language::MetadataSections *result = nullptr;

  auto snapshot = language::registered->snapshot();
  if (index < snapshot.count()) {
    result = snapshot[index];
  }

  if (result) {
    // Ensure the base address is fixed up before returning it.
    language::fixupMetadataSectionBaseAddress(result);
  }

  return result;
}

LANGUAGE_RUNTIME_EXPORT
const char *
language_getMetadataSectionName(const language::MetadataSections *section) {
  if (auto info = language::SymbolInfo::lookup(section)) {
    if (info->getFilename()) {
      return info->getFilename();
    }
  }
  return "";
}

LANGUAGE_RUNTIME_EXPORT
void language_getMetadataSectionBaseAddress(const language::MetadataSections *section,
                                         void const **out_actual,
                                         void const **out_expected) {
  if (auto info = language::SymbolInfo::lookup(section)) {
    *out_actual = info->getBaseAddress();
  } else {
    *out_actual = nullptr;
  }

  // fixupMetadataSectionBaseAddress() was already called by
  // language_getMetadataSection(), presumably on the same thread, so we don't need
  // to call it again here.
  *out_expected = section->baseAddress.load(std::memory_order_relaxed);
}

LANGUAGE_RUNTIME_EXPORT
size_t language_getMetadataSectionCount() {
  auto snapshot = language::registered->snapshot();
  return snapshot.count();
}

#endif // NDEBUG

#endif // !defined(__MACH__)

#endif // LANGUAGE_RUNTIME_IMAGEINSPECTIONCOMMON_H
