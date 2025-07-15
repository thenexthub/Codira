//===--- MetadataSections.h -----------------------------------------------===//
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
/// This file contains the declaration of the MetadataSectionsRange and 
/// MetadataSections struct, which represent, respectively,  information about  
/// an image's section, and an image's metadata information (which is composed
/// of multiple section information).
///
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_STDLIB_SHIMS_METADATASECTIONS_H
#define LANGUAGE_STDLIB_SHIMS_METADATASECTIONS_H

#if defined(__cplusplus) && !defined(__language__)
#include <atomic>
#endif

#include "CodiraStddef.h"
#include "CodiraStdint.h"

#ifdef __cplusplus
namespace language { 
extern "C" {
#endif

/// Specifies the address range corresponding to a section.
typedef struct MetadataSectionRange {
  __language_uintptr_t start;
  __language_size_t length;
} MetadataSectionRange;


/// Identifies the address space ranges for the Codira metadata required by the
/// Codira runtime.
///
/// \warning If you change the size of this structure by adding fields, it is an
///   ABI-breaking change on platforms that use it. Make sure to increment
///   \c CurrentSectionMetadataVersion if you do. To minimize impact, always add
///   new fields to the \em end of the structure.
struct MetadataSections {
  __language_uintptr_t version;

  /// The base address of the image where this metadata section was defined, as
  /// reported when the section was registered with the Codira runtime.
  ///
  /// The value of this field is equivalent to the value of
  /// \c SymbolInfo::baseAddress as returned from \c SymbolInfo::lookup() for a
  /// symbol in the image that contains these sections.
  ///
  /// For Mach-O images, set this field to \c __dso_handle (i.e. the Mach header
  /// for the image.) For ELF images, set it to \c __dso_handle (the runtime
  /// will adjust it to the start of the ELF image when the image is loaded.)
  /// For COFF images, set this field to \c __ImageBase.
  ///
  /// For platforms that have a single statically-linked image or no dynamic
  /// loader (i.e. no equivalent of \c __dso_handle or \c __ImageBase), this
  /// field is ignored and should be set to \c nullptr.
  ///
  /// \bug When imported into Codira, this field is not atomic.
  ///
  /// \sa language_addNewDSOImage()
#if defined(__language__) || defined(__STDC_NO_ATOMICS__)
  const void *baseAddress;
#elif defined(__cplusplus)
  std::atomic<const void *> baseAddress;
#else
  _Atomic(const void *) baseAddress;
#endif

  /// Unused.
  ///
  /// These pointers (or the space they occupy) can be repurposed without
  /// causing ABI breakage. Set them to \c nullptr.
  void *unused0;
  void *unused1;

  MetadataSectionRange language5_protocols;
  MetadataSectionRange language5_protocol_conformances;
  MetadataSectionRange language5_type_metadata;
  MetadataSectionRange language5_typeref;
  MetadataSectionRange language5_reflstr;
  MetadataSectionRange language5_fieldmd;
  MetadataSectionRange language5_assocty;
  MetadataSectionRange language5_replace;
  MetadataSectionRange language5_replac2;
  MetadataSectionRange language5_builtin;
  MetadataSectionRange language5_capture;
  MetadataSectionRange language5_mpenum;
  MetadataSectionRange language5_accessible_functions;
  MetadataSectionRange language5_runtime_attributes;
  MetadataSectionRange language5_tests;
};

#ifdef __cplusplus
} //extern "C"
} // namespace language
#endif

#endif // LANGUAGE_STDLIB_SHIMS_METADATASECTIONS_H
