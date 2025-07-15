//===--- CodiraRemoteMirrorTypes.h - Remote reflection types -----*- C++ -*-===//
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
/// This header declares types in the liblanguageReflection library.
///
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_REMOTE_MIRROR_TYPES_H
#define LANGUAGE_REMOTE_MIRROR_TYPES_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Pointers used here need to be pointer-sized on watchOS for binary
// compatibility. Everywhere else, they are 64-bit so 32-bit processes can
// potentially read from 64-bit processes.
#if defined(__APPLE__) && defined(__MACH__)
#include <TargetConditionals.h>
#if TARGET_OS_WATCH
#define LANGUAGE_REFLECTION_NATIVE_POINTERS 1
#endif
#endif

#if LANGUAGE_REFLECTION_NATIVE_POINTERS
typedef uintptr_t language_reflection_ptr_t;
#else
typedef uint64_t language_reflection_ptr_t;
#endif

typedef language_reflection_ptr_t language_typeref_t;

/// Represents one of the Codira reflection sections of an image.
typedef struct language_reflection_section {
  void *Begin;
  void *End;
} language_reflection_section_t;

/// Represents the remote address and size of an image's section
typedef struct language_remote_reflection_section {
    uintptr_t StartAddress;
    uintptr_t Size;
} language_remote_reflection_section_t;

typedef struct language_reflection_section_pair {
  language_reflection_section_t section;
  language_reflection_ptr_t offset; ///< DEPRECATED. Must be zero
} language_reflection_section_pair_t;

/// Represents the mapping between an image sections's local and remote addresses
typedef struct language_reflection_section_mapping {
  language_reflection_section_t local_section;
  language_remote_reflection_section_t remote_section;
} language_reflection_section_mapping_t;

/// Represents the set of Codira reflection sections of an image.
/// Not all sections may be present.
///
/// DEPRECATED. New RemoteMirror clients should use
/// \c language_reflection_addImage .
typedef struct language_reflection_info {
  language_reflection_section_pair_t field;
  language_reflection_section_pair_t associated_types;
  language_reflection_section_pair_t builtin_types;
  language_reflection_section_pair_t capture;
  language_reflection_section_pair_t type_references;
  language_reflection_section_pair_t reflection_strings;

  // Start address in local and remote address spaces.
  language_reflection_ptr_t LocalStartAddress;
  language_reflection_ptr_t RemoteStartAddress;
} language_reflection_info_t;

/// Represents the set of Codira reflection sections of an image,
typedef struct language_reflection_mapping_info {
  language_reflection_section_mapping_t field;
  language_reflection_section_mapping_t associated_types;
  language_reflection_section_mapping_t builtin_types;
  language_reflection_section_mapping_t capture;
  language_reflection_section_mapping_t type_references;
  language_reflection_section_mapping_t reflection_strings;
  // New fields cannot be added here without breaking ABI.
  // Use language_reflection_addImage instead.
} language_reflection_mapping_info_t;

/// The layout kind of a Codira type.
typedef enum language_layout_kind {
  // Nothing is known about the size or contents of this value.
  LANGUAGE_UNKNOWN,

  // An opaque value with known size and alignment but no specific
  // interpretation.
  LANGUAGE_BUILTIN,

  // A pointer-size value that is known to contain the address of
  // another heap allocation, or NULL.
  LANGUAGE_RAW_POINTER,

  // Value types consisting of zero or more fields.
  LANGUAGE_TUPLE,
  LANGUAGE_STRUCT,

  // An enum with no payload cases. The record will have no fields, but
  // will have the correct size.
  LANGUAGE_NO_PAYLOAD_ENUM,

  // An enum with a single payload case. The record consists of a single
  // field, being the enum payload.
  LANGUAGE_SINGLE_PAYLOAD_ENUM,

  // An enum with multiple payload cases. The record consists of a multiple
  // fields, one for each enum payload.
  LANGUAGE_MULTI_PAYLOAD_ENUM,

  LANGUAGE_THICK_FUNCTION,

  LANGUAGE_OPAQUE_EXISTENTIAL,
  LANGUAGE_CLASS_EXISTENTIAL,
  LANGUAGE_ERROR_EXISTENTIAL,
  LANGUAGE_EXISTENTIAL_METATYPE,

  // References to other objects in the heap.
  LANGUAGE_STRONG_REFERENCE,
  LANGUAGE_UNOWNED_REFERENCE,
  LANGUAGE_WEAK_REFERENCE,
  LANGUAGE_UNMANAGED_REFERENCE,

  // Layouts of heap objects. These are only ever returned from
  // language_reflection_infoFor{Instance,Metadata}(), and not
  // language_reflection_infoForTypeRef().
  LANGUAGE_CLASS_INSTANCE,
  LANGUAGE_CLOSURE_CONTEXT,

  // A contiguous list of N Ts, typically for Builtin.FixedArray<N, T>.
  LANGUAGE_ARRAY,
} language_layout_kind_t;

struct language_childinfo;

/// A description of the memory layout of a type or field of a type.
typedef struct language_typeinfo {
  language_layout_kind_t Kind;

  unsigned Size;
  unsigned Alignment;
  unsigned Stride;

  unsigned NumFields;
} language_typeinfo_t;

typedef struct language_childinfo {
  /// The memory for Name is owned by the reflection context.
  const char *Name;
  unsigned Offset;
  language_layout_kind_t Kind;
  language_typeref_t TR;
} language_childinfo_t;

// Values here match the values from MetadataAllocatorTags in Metadata.h.
enum language_metadata_allocation_tag {
  LANGUAGE_GENERIC_METADATA_CACHE_ALLOCATION = 14,
};

typedef int language_metadata_allocation_tag_t;

/// A metadata allocation made by the Codira runtime.
typedef struct language_metadata_allocation {
  /// The allocation's tag, which describes what kind of allocation it is. This
  /// may be one of the values in language_metadata_allocation_tag, or something
  /// else, in which case the tag should be considered unknown.
  language_metadata_allocation_tag_t Tag;

  /// A pointer to the start of the allocation in the remote process.
  language_reflection_ptr_t Ptr;

  /// The size of the allocation in bytes.
  unsigned Size;
} language_metadata_allocation_t;

typedef struct language_metadata_cache_node {
  language_reflection_ptr_t Left;
  language_reflection_ptr_t Right;
} language_metadata_cache_node_t;

/// The return value when getting an async task's slab pointer.
typedef struct language_async_task_slab_return {
  /// On failure, a pointer to a string describing the error. On success, NULL.
  /// This pointer remains valid until the next
  /// language_reflection call on the given context.
  const char *Error;

  /// The task's slab pointer, if no error occurred.
  language_reflection_ptr_t SlabPtr;
} language_async_task_slab_return_t;

typedef struct language_async_task_allocation_chunk {
  language_reflection_ptr_t Start;
  unsigned Length;
  language_layout_kind_t Kind;
} language_async_task_allocation_chunk_t;

typedef struct language_async_task_slab_allocations_return {
  /// On failure, a pointer to a string describing the error. On success, NULL.
  /// This pointer remains valid until the next
  /// language_reflection call on the given context.
  const char *Error;

  /// The remote pointer to the next slab, or NULL/0 if none.
  language_reflection_ptr_t NextSlab;

  /// The size of the entire slab, in bytes.
  unsigned SlabSize;

  /// The number of chunks pointed to by Chunks.
  unsigned ChunkCount;

  /// A pointer to the chunks, if no error occurred.
  language_async_task_allocation_chunk_t *Chunks;
} language_async_task_slab_allocations_return_t;

typedef struct language_async_task_info {
  /// On failure, a pointer to a string describing the error. On success, NULL.
  /// This pointer remains valid until the next
  /// language_reflection call on the given context.
  const char *Error;

  unsigned Kind;
  unsigned EnqueuePriority;
  bool IsChildTask;
  bool IsFuture;
  bool IsGroupChildTask;
  bool IsAsyncLetTask;
  bool IsSynchronousStartTask;

  unsigned MaxPriority;
  bool IsCancelled;
  bool IsStatusRecordLocked;
  bool IsEscalated;
  bool HasIsRunning; // If false, the IsRunning flag is not valid.
  bool IsRunning;
  bool IsEnqueued;

  bool HasThreadPort;
  uint32_t ThreadPort;

  uint64_t Id;
  language_reflection_ptr_t RunJob;
  language_reflection_ptr_t AllocatorSlabPtr;

  unsigned ChildTaskCount;
  language_reflection_ptr_t *ChildTasks;

  unsigned AsyncBacktraceFramesCount;
  language_reflection_ptr_t *AsyncBacktraceFrames;
} language_async_task_info_t;

typedef struct language_actor_info {
  /// On failure, a pointer to a string describing the error. On success, NULL.
  /// This pointer remains valid until the next
  /// language_reflection call on the given context.
  const char *Error;

  uint8_t State;
  bool IsDistributedRemote;
  bool IsPriorityEscalated;
  uint8_t MaxPriority;

  language_reflection_ptr_t FirstJob;

  bool HasThreadPort;
  uint32_t ThreadPort;
} language_actor_info_t;

/// An opaque pointer to a context which maintains state and
/// caching of reflection structure for heap instances.
typedef struct CodiraReflectionContext *CodiraReflectionContextRef;

#ifdef __cplusplus
} // extern "C"
#endif

#endif // LANGUAGE_REMOTE_MIRROR_TYPES_H
