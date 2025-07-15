//===--- CodiraRemoteMirror.h - Public remote reflection interf. -*- C++ -*-===//
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
/// This header declares functions in the liblanguageReflection library,
/// which provides mechanisms for reflecting heap information in a
/// remote Codira process.
///
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_REMOTE_MIRROR_H
#define LANGUAGE_REMOTE_MIRROR_H

#include "Platform.h"
#include "MemoryReaderInterface.h"
#include "CodiraRemoteMirrorTypes.h"

#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

LANGUAGE_REMOTE_MIRROR_LINKAGE
#if !defined(_WIN32)
__attribute__((__weak_import__))
#endif
extern unsigned long long language_reflection_classIsCodiraMask;

/// An arbitrary version number for this library. Incremented to indicate the
/// presence of a bug fix or feature that can't be detected from the outside
/// otherwise. The currently used version numbers are:
///
/// 0 - Indicates that language_reflection_iterateAsyncTaskAllocations has the
///     first attempted fix to use the right AsyncTask layout.
/// 1 - Indicates that language_reflection_iterateAsyncTaskAllocations has been
///     actually fixed to use the right AsyncTask layout.
/// 2 - language_reflection_iterateAsyncTaskAllocations has been replaced by
///     language_reflection_asyncTaskSlabPointer and
///     language_reflection_asyncTaskSlabAllocations.
/// 3 - The async task slab size calculation is fixed to account for alignment,
///     no longer short by 8 bytes.
LANGUAGE_REMOTE_MIRROR_LINKAGE extern uint32_t language_reflection_libraryVersion;

/// Get the metadata version supported by the Remote Mirror library.
LANGUAGE_REMOTE_MIRROR_LINKAGE
uint16_t language_reflection_getSupportedMetadataVersion(void);

/// \returns An opaque reflection context.
LANGUAGE_REMOTE_MIRROR_LINKAGE
CodiraReflectionContextRef
language_reflection_createReflectionContext(void *ReaderContext,
                                         uint8_t PointerSize,
                                         FreeBytesFunction Free,
                                         ReadBytesFunction ReadBytes,
                                         GetStringLengthFunction GetStringLength,
                                         GetSymbolAddressFunction GetSymbolAddress);

/// \returns An opaque reflection context.
LANGUAGE_REMOTE_MIRROR_LINKAGE
CodiraReflectionContextRef
language_reflection_createReflectionContextWithDataLayout(void *ReaderContext,
                                     QueryDataLayoutFunction DataLayout,
                                     FreeBytesFunction Free,
                                     ReadBytesFunction ReadBytes,
                                     GetStringLengthFunction GetStringLength,
                                     GetSymbolAddressFunction GetSymbolAddress);

/// Destroys an opaque reflection context.
LANGUAGE_REMOTE_MIRROR_LINKAGE
void
language_reflection_destroyReflectionContext(CodiraReflectionContextRef Context);

/// DEPRECATED. Add reflection sections for a loaded Codira image.
///
/// You probably want to use \c language_reflection_addImage instead.
LANGUAGE_REMOTE_MIRROR_LINKAGE
void
language_reflection_addReflectionInfo(CodiraReflectionContextRef ContextRef,
                                   language_reflection_info_t Info);

/// Add reflection sections from a loaded Codira image.
LANGUAGE_REMOTE_MIRROR_LINKAGE
void language_reflection_addReflectionMappingInfo(
    CodiraReflectionContextRef ContextRef, language_reflection_mapping_info_t Info);

/// Add reflection information from a loaded Codira image.
/// Returns true on success, false if the image's memory couldn't be read.
LANGUAGE_REMOTE_MIRROR_LINKAGE
int
language_reflection_addImage(CodiraReflectionContextRef ContextRef,
                          language_addr_t imageStart);

/// Returns a boolean indicating if the isa mask was successfully
/// read, in which case it is stored in the isaMask out parameter.
LANGUAGE_REMOTE_MIRROR_LINKAGE
int language_reflection_readIsaMask(CodiraReflectionContextRef ContextRef,
                                 uintptr_t *outIsaMask);

/// Returns an opaque type reference for a metadata pointer, or
/// NULL if one can't be constructed.
///
/// This function loses information; in particular, passing the
/// result to language_reflection_infoForTypeRef() will not give
/// the same result as calling language_reflection_infoForMetadata().
LANGUAGE_REMOTE_MIRROR_LINKAGE
language_typeref_t
language_reflection_typeRefForMetadata(CodiraReflectionContextRef ContextRef,
                                    uintptr_t Metadata);

/// Returns whether the given object appears to have metadata understood
/// by this library. Images must be added using
/// language_reflection_addImage, not language_reflection_addReflectionInfo,
/// for this function to work properly. If addImage is used, a negative
/// result is always correct, but a positive result may be a false
/// positive if the address in question is not really a Codira or
/// Objective-C object. If addReflectionInfo is used, the return value
/// will always be false.
LANGUAGE_REMOTE_MIRROR_LINKAGE
int
language_reflection_ownsObject(CodiraReflectionContextRef ContextRef, uintptr_t Object);

/// Returns whether the given address is associated with an image added to this
/// library. Images must be added using language_reflection_addImage, not
/// language_reflection_addReflectionInfo, for this function to work
/// properly. If addReflectionInfo is used, the return value will always
/// be false.
LANGUAGE_REMOTE_MIRROR_LINKAGE
int
language_reflection_ownsAddress(CodiraReflectionContextRef ContextRef, uintptr_t Address);

/// Returns whether the given address is strictly in the DATA, AUTH or TEXT
/// segments of the image added to this library. Images must be added using
/// language_reflection_addImage, not language_reflection_addReflectionInfo, for this
/// function to work properly. If addReflectionInfo is used, the return value
/// will always be false.
LANGUAGE_REMOTE_MIRROR_LINKAGE
int
language_reflection_ownsAddressStrict(CodiraReflectionContextRef ContextRef, uintptr_t Address);

/// Returns the metadata pointer for a given object.
LANGUAGE_REMOTE_MIRROR_LINKAGE
uintptr_t
language_reflection_metadataForObject(CodiraReflectionContextRef ContextRef,
                                   uintptr_t Object);

/// Returns the nominal type descriptor given the metadata
LANGUAGE_REMOTE_MIRROR_LINKAGE
language_reflection_ptr_t
language_reflection_metadataNominalTypeDescriptor(CodiraReflectionContextRef ContextRef,
																							 language_reflection_ptr_t Metadata);


LANGUAGE_REMOTE_MIRROR_LINKAGE
int
language_reflection_metadataIsActor(CodiraReflectionContextRef ContextRef,
                                 language_reflection_ptr_t Metadata);

/// Returns an opaque type reference for a class or closure context
/// instance pointer, or NULL if one can't be constructed.
///
/// This function loses information; in particular, passing the
/// result to language_reflection_infoForTypeRef() will not give
/// the same result as calling language_reflection_infoForInstance().
LANGUAGE_REMOTE_MIRROR_LINKAGE
language_typeref_t
language_reflection_typeRefForInstance(CodiraReflectionContextRef ContextRef,
                                    uintptr_t Object);

/// Returns a typeref from a mangled type name string.
LANGUAGE_REMOTE_MIRROR_LINKAGE
language_typeref_t
language_reflection_typeRefForMangledTypeName(CodiraReflectionContextRef ContextRef,
                                           const char *MangledName,
                                           uint64_t Length);

/// Returns the demangled name for a typeref, or NULL if the name couldn't be
/// created.
///
/// The returned string is heap allocated and the caller must free() it when
/// done.
LANGUAGE_REMOTE_MIRROR_DEPRECATED(
    "Please use language_reflection_copyNameForTypeRef()",
    "language_reflection_copyNameForTypeRef")
LANGUAGE_REMOTE_MIRROR_LINKAGE
char *
language_reflection_copyDemangledNameForTypeRef(
  CodiraReflectionContextRef ContextRef, language_typeref_t OpaqueTypeRef);

LANGUAGE_REMOTE_MIRROR_LINKAGE
char *
language_reflection_copyDemangledNameForProtocolDescriptor(
  CodiraReflectionContextRef ContextRef, language_reflection_ptr_t Proto);

/// Returns the mangled or demangled name for a typeref, or NULL if the name
/// couldn't be created.
///
/// The returned string is heap allocated and the caller must free() it when
/// done.

LANGUAGE_REMOTE_MIRROR_LINKAGE
char*
language_reflection_copyNameForTypeRef(CodiraReflectionContextRef ContextRef,
                                    language_typeref_t OpaqueTypeRef,
                                    bool mangled);

/// Returns a structure describing the layout of a value of a typeref.
/// For classes, this returns the reference value itself.
LANGUAGE_REMOTE_MIRROR_LINKAGE
language_typeinfo_t
language_reflection_infoForTypeRef(CodiraReflectionContextRef ContextRef,
                                language_typeref_t OpaqueTypeRef);

/// Returns the information about a child field of a value by index.
LANGUAGE_REMOTE_MIRROR_LINKAGE
language_childinfo_t
language_reflection_childOfTypeRef(CodiraReflectionContextRef ContextRef,
                                language_typeref_t OpaqueTypeRef, unsigned Index);

/// Returns a structure describing the layout of a class instance
/// from the isa pointer of a class.
LANGUAGE_REMOTE_MIRROR_LINKAGE
language_typeinfo_t
language_reflection_infoForMetadata(CodiraReflectionContextRef ContextRef,
                                 uintptr_t Metadata);

/// Returns the information about a child field of a class instance
/// by index.
LANGUAGE_REMOTE_MIRROR_LINKAGE
language_childinfo_t
language_reflection_childOfMetadata(CodiraReflectionContextRef ContextRef,
                                 uintptr_t Metadata, unsigned Index);

/// Returns a structure describing the layout of a class or closure
/// context instance, from a pointer to the object itself.
LANGUAGE_REMOTE_MIRROR_LINKAGE
language_typeinfo_t
language_reflection_infoForInstance(CodiraReflectionContextRef ContextRef,
                                 uintptr_t Object);

/// Returns the information about a child field of a class or closure
/// context instance.
LANGUAGE_REMOTE_MIRROR_LINKAGE
language_childinfo_t
language_reflection_childOfInstance(CodiraReflectionContextRef ContextRef,
                                 uintptr_t Object, unsigned Index);

/// Returns the number of generic arguments of a typeref.
LANGUAGE_REMOTE_MIRROR_LINKAGE
unsigned
language_reflection_genericArgumentCountOfTypeRef(language_typeref_t OpaqueTypeRef);

/// Returns a fully instantiated typeref for a generic argument by index.
LANGUAGE_REMOTE_MIRROR_LINKAGE
language_typeref_t
language_reflection_genericArgumentOfTypeRef(language_typeref_t OpaqueTypeRef,
                                          unsigned Index);

/// Projects the type inside of an existential container.
///
/// Takes the address of the start of an existential container and the typeref
/// for the existential, and sets two out parameters:
///
/// - InstanceTypeRef: A type reference for the type inside of the existential
///   container.
/// - StartOfInstanceData: The address to the start of the inner type's instance
///   data, which may or may not be inside the container directly.
///   If the type is a reference type, the pointer to the instance is the first
///   word in the container.
///
///   If the existential contains a value type that can fit in the first three
///   spare words of the container, *StartOfInstanceData == InstanceAddress.
///   If it's a value type that can't fit in three words, the data will be in
///   its own heap box, so *StartOfInstanceData will be the address of that box.
///
/// Returns true if InstanceTypeRef and StartOfInstanceData contain valid
/// valid values.
LANGUAGE_REMOTE_MIRROR_LINKAGE
int language_reflection_projectExistential(CodiraReflectionContextRef ContextRef,
                                        language_addr_t ExistentialAddress,
                                        language_typeref_t ExistentialTypeRef,
                                        language_typeref_t *OutInstanceTypeRef,
                                        language_addr_t *OutStartOfInstanceData);

/// Like language_reflection_projectExistential, with 2 differences:
///
/// - When dealing with an error existential, this version will dereference 
///   the ExistentialAddress before proceeding.
/// - After setting OutInstanceTypeRef and OutStartOfInstanceData this version
///   may dereference and set OutStartOfInstanceData if OutInstanceTypeRef is a 
///   class TypeRef.
LANGUAGE_REMOTE_MIRROR_LINKAGE
int language_reflection_projectExistentialAndUnwrapClass(
    CodiraReflectionContextRef ContextRef, language_addr_t ExistentialAddress,
    language_typeref_t ExistentialTypeRef, language_typeref_t *OutInstanceTypeRef,
    language_addr_t *OutStartOfInstanceData);

/// Projects the value of an enum.
///
/// Takes the address and typeref for an enum and determines the
/// index of the currently-selected case within the enum.
/// You can use this index with `language_reflection_childOfTypeRef`
/// to get detailed information about the specific case.
///
/// Returns true if the enum case could be successfully determined.
/// In particular, note that this code may fail for valid in-memory data
/// if the compiler is using a strategy we do not yet understand.
LANGUAGE_REMOTE_MIRROR_LINKAGE
int language_reflection_projectEnumValue(CodiraReflectionContextRef ContextRef,
                                      language_addr_t EnumAddress,
                                      language_typeref_t EnumTypeRef,
                                      int *CaseIndex);

/// Dump a brief description of the typeref as a tree to stderr.
LANGUAGE_REMOTE_MIRROR_LINKAGE
void language_reflection_dumpTypeRef(language_typeref_t OpaqueTypeRef);

/// Dump information about the layout for a typeref.
LANGUAGE_REMOTE_MIRROR_LINKAGE
void language_reflection_dumpInfoForTypeRef(CodiraReflectionContextRef ContextRef,
                                         language_typeref_t OpaqueTypeRef);

/// Dump information about the layout of a class instance from its isa pointer.
LANGUAGE_REMOTE_MIRROR_LINKAGE
void language_reflection_dumpInfoForMetadata(CodiraReflectionContextRef ContextRef,
                                          uintptr_t Metadata);

/// Dump information about the layout of a class or closure context instance.
LANGUAGE_REMOTE_MIRROR_LINKAGE
void language_reflection_dumpInfoForInstance(CodiraReflectionContextRef ContextRef,
                                          uintptr_t Object);

/// Demangle a type name.
///
/// Copies at most `MaxLength` bytes from the demangled name string into
/// `OutDemangledName`.
///
/// Returns the length of the demangled string this function tried to copy
/// into `OutDemangledName`.
LANGUAGE_REMOTE_MIRROR_LINKAGE
size_t language_reflection_demangle(const char *MangledName, size_t Length,
                                 char *OutDemangledName, size_t MaxLength);

/// Iterate over the process's protocol conformance cache.
///
/// Calls the passed in Call function for each protocol conformance found in
/// the conformance cache. The function is passed the type which conforms and
/// the protocol it conforms to. The ContextPtr is passed through unchanged.
///
/// Returns NULL on success. On error, returns a pointer to a C string
/// describing the error. This pointer remains valid until the next
/// language_reflection call on the given context.
LANGUAGE_REMOTE_MIRROR_LINKAGE
const char *language_reflection_iterateConformanceCache(
  CodiraReflectionContextRef ContextRef,
  void (*Call)(language_reflection_ptr_t Type,
               language_reflection_ptr_t Proto,
               void *ContextPtr),
  void *ContextPtr);

/// Iterate over the process's metadata allocations.
///
/// Calls the passed in Call function for each metadata allocation. The function
/// is passed a structure that describes the allocation. The ContextPtr is
/// passed through unchanged.
///
/// Returns NULL on success. On error, returns a pointer to a C string
/// describing the error. This pointer remains valid until the next
/// language_reflection call on the given context.
LANGUAGE_REMOTE_MIRROR_LINKAGE
const char *language_reflection_iterateMetadataAllocations(
  CodiraReflectionContextRef ContextRef,
  void (*Call)(language_metadata_allocation_t Allocation,
               void *ContextPtr),
  void *ContextPtr);

/// Given a metadata allocation, return the metadata it points to. Returns NULL
/// on failure. Despite the name, not all allocations point to metadata.
/// Currently, this will return a metadata only for allocations with tag
/// GenericMetadataCache. Support for additional tags may be added in the
/// future. The caller must gracefully handle failure.
LANGUAGE_REMOTE_MIRROR_LINKAGE
language_reflection_ptr_t language_reflection_allocationMetadataPointer(
  CodiraReflectionContextRef ContextRef,
  language_metadata_allocation_t Allocation);

/// Return the name of a metadata allocation tag, or NULL if the tag is unknown.
/// This pointer remains valid until the next language_reflection call on the given
/// context.
LANGUAGE_REMOTE_MIRROR_LINKAGE
const char *
language_reflection_metadataAllocationTagName(CodiraReflectionContextRef ContextRef,
                                           language_metadata_allocation_tag_t Tag);

LANGUAGE_REMOTE_MIRROR_LINKAGE
int language_reflection_metadataAllocationCacheNode(
    CodiraReflectionContextRef ContextRef,
    language_metadata_allocation_t Allocation,
    language_metadata_cache_node_t *OutNode);

/// Backtrace iterator callback passed to
/// language_reflection_iterateMetadataAllocationBacktraces
typedef void (*language_metadataAllocationBacktraceIterator)(
    language_reflection_ptr_t AllocationPtr, size_t Count,
    const language_reflection_ptr_t Ptrs[], void *ContextPtr);

/// Iterate over all recorded metadata allocation backtraces in the process.
///
/// Calls the passed in Call function for each recorded backtrace. The function
/// is passed the number of backtrace entries and an array of those entries, as
/// pointers. The array is stored from deepest to shallowest, so main() will be
/// somewhere near the end. This array is valid only for the duration of the
/// call.
///
/// Returns NULL on success. On error, returns a pointer to a C string
/// describing the error. This pointer remains valid until the next
/// language_reflection call on the given context.
LANGUAGE_REMOTE_MIRROR_LINKAGE
const char *language_reflection_iterateMetadataAllocationBacktraces(
    CodiraReflectionContextRef ContextRef,
    language_metadataAllocationBacktraceIterator Call, void *ContextPtr);

/// Get the first allocation slab for a given async task object.
/// This object must have an isa value equal to
/// _language_concurrency_debug_asyncTaskMetadata.
///
/// It is possible that the async task object hasn't allocated a slab yet, in
/// which case the slab pointer will be NULL. If non-NULL, the returned slab
/// pointer may be a separate heap allocation, or it may be interior to some
/// allocation used by the task.
LANGUAGE_REMOTE_MIRROR_LINKAGE
language_async_task_slab_return_t
language_reflection_asyncTaskSlabPointer(CodiraReflectionContextRef ContextRef,
                                      language_reflection_ptr_t AsyncTaskPtr);

/// Iterate over the allocations in the given async task allocator slab.
/// This allocation must have an "isa" value (scare quotes because it's not a
/// real object) equal to _language_concurrency_debug_asyncTaskSlabMetadata.
///
/// Calls the passed in Call function for each allocation in the slab. The
/// function is passed the allocation pointer and an array of chunks. Each chunk
/// consists of a start, length, and kind for that chunk of the allocated
/// memory. Any regions of the allocation that are not covered by a chunk are
/// unallocated or garbage. The chunk array is valid only for the duration of
/// the call.
///
/// A slab may be part of a chain of slabs, so the
/// function may be called more than once.
///
/// Returns NULL on success. On error, returns a pointer to a C string
/// describing the error. This pointer remains valid until the next
/// language_reflection call on the given context.
LANGUAGE_REMOTE_MIRROR_LINKAGE
language_async_task_slab_allocations_return_t
language_reflection_asyncTaskSlabAllocations(CodiraReflectionContextRef ContextRef,
                                          language_reflection_ptr_t SlabPtr);

LANGUAGE_REMOTE_MIRROR_LINKAGE
language_async_task_info_t
language_reflection_asyncTaskInfo(CodiraReflectionContextRef ContextRef,
                               language_reflection_ptr_t AsyncTaskPtr);

LANGUAGE_REMOTE_MIRROR_LINKAGE
language_actor_info_t
language_reflection_actorInfo(CodiraReflectionContextRef ContextRef,
                           language_reflection_ptr_t ActorPtr);

LANGUAGE_REMOTE_MIRROR_LINKAGE
language_reflection_ptr_t
language_reflection_nextJob(CodiraReflectionContextRef ContextRef,
                         language_reflection_ptr_t JobPtr);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // LANGUAGE_REFLECTION_LANGUAGE_REFLECTION_H

