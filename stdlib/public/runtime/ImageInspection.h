//===--- ImageInspection.h - Image inspection routines ----------*- C++ -*-===//
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
/// This file includes routines that extract metadata from executable and
/// dynamic library image files generated by the Codira compiler. The concrete
/// implementations vary greatly by platform.
///
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_RUNTIME_IMAGEINSPECTION_H
#define LANGUAGE_RUNTIME_IMAGEINSPECTION_H

#include "language/Runtime/Config.h"

#include <cstdint>
#include <cstddef>
#include <functional>
#include <memory>
#include <type_traits>

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#endif

#include "SymbolInfo.h"

namespace language {

/// Load the metadata from the image necessary to find protocols by name.
void initializeProtocolLookup();

/// Load the metadata from the image necessary to find a type's
/// protocol conformance.
void initializeProtocolConformanceLookup();

/// Load the metadata from the image necessary to find a type by name.
void initializeTypeMetadataRecordLookup();

/// Load the metadata from the image necessary to perform dynamic replacements.
void initializeDynamicReplacementLookup();

/// Load the metadata from the image necessary to find functions by name.
void initializeAccessibleFunctionsLookup();

// Callbacks to register metadata from an image to the runtime.
void addImageProtocolsBlockCallback(const void *baseAddress,
                                    const void *start, uintptr_t size);
void addImageProtocolsBlockCallbackUnsafe(const void *baseAddress,
                                          const void *start, uintptr_t size);
void addImageProtocolConformanceBlockCallback(const void *baseAddress,
                                              const void *start,
                                              uintptr_t size);
void addImageProtocolConformanceBlockCallbackUnsafe(const void *baseAddress,
                                                    const void *start,
                                                    uintptr_t size);
void addImageTypeMetadataRecordBlockCallback(const void *baseAddress,
                                             const void *start,
                                             uintptr_t size);
void addImageTypeMetadataRecordBlockCallbackUnsafe(const void *baseAddress,
                                                   const void *start,
                                                   uintptr_t size);
void addImageDynamicReplacementBlockCallback(const void *baseAddress,
                                             const void *start, uintptr_t size,
                                             const void *start2,
                                             uintptr_t size2);

void addImageAccessibleFunctionsBlockCallback(const void *baseAddress,
                                              const void *start,
                                              uintptr_t size);
void addImageAccessibleFunctionsBlockCallbackUnsafe(const void *baseAddress,
                                                    const void *start,
                                                    uintptr_t size);

} // end namespace language

#endif
