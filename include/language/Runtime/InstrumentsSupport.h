//===--- InstrumentsSupport.h - Support for Instruments.app -----*- C++ -*-===//
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
// Codira runtime support for instruments.app
// In the long run, they plan to use dyld to make this indirection go away.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_RUNTIME_INSTRUMENTS_SUPPORT_H
#define LANGUAGE_RUNTIME_INSTRUMENTS_SUPPORT_H

#define LANGUAGE_RT_DECLARE_ENTRY \
  __ptrauth_language_runtime_function_entry

namespace language {

#ifdef LANGUAGE_STDLIB_OVERRIDABLE_RETAIN_RELEASE

// liboainject patches the function pointers and calls the functions below.
LANGUAGE_RUNTIME_EXPORT
HeapObject *(*LANGUAGE_RT_DECLARE_ENTRY _language_allocObject)(
                                  HeapMetadata const *metadata,
                                  size_t requiredSize,
                                  size_t requiredAlignmentMask);
LANGUAGE_RUNTIME_EXPORT
HeapObject *(*LANGUAGE_RT_DECLARE_ENTRY _language_retain)(HeapObject *object);
LANGUAGE_RUNTIME_EXPORT
HeapObject *(*LANGUAGE_RT_DECLARE_ENTRY _language_retain_n)(HeapObject *object, uint32_t n);
LANGUAGE_RUNTIME_EXPORT
HeapObject *(*LANGUAGE_RT_DECLARE_ENTRY _language_tryRetain)(HeapObject *object);
LANGUAGE_RUNTIME_EXPORT
void (*LANGUAGE_RT_DECLARE_ENTRY _language_release)(HeapObject *object);
LANGUAGE_RUNTIME_EXPORT
void (*LANGUAGE_RT_DECLARE_ENTRY _language_release_n)(HeapObject *object, uint32_t n);
LANGUAGE_RUNTIME_EXPORT
std::atomic<bool> _language_enableSwizzlingOfAllocationAndRefCountingFunctions_forInstrumentsOnly;
LANGUAGE_RUNTIME_EXPORT
size_t language_retainCount(HeapObject *object);

// liboainject tries to patch the function pointers and call the functions below
// Codira used to implement these but no longer does.
// Do not reuse these names unless you do what oainject expects you to do.
LANGUAGE_RUNTIME_EXPORT
void *(*_language_alloc)(size_t idx);
LANGUAGE_RUNTIME_EXPORT
void *(*_language_tryAlloc)(size_t idx);
LANGUAGE_RUNTIME_EXPORT
void *(*_language_slowAlloc)(size_t bytes, size_t alignMask, uintptr_t flags);
LANGUAGE_RUNTIME_EXPORT
void (*_language_dealloc)(void *ptr, size_t idx);
LANGUAGE_RUNTIME_EXPORT
void (*_language_slowDealloc)(void *ptr, size_t bytes, size_t alignMask);
LANGUAGE_RUNTIME_EXPORT
size_t _language_indexToSize(size_t idx);
LANGUAGE_RUNTIME_EXPORT
void _language_zone_init(void);

#endif // LANGUAGE_STDLIB_OVERRIDABLE_RETAIN_RELEASE

}

#endif
