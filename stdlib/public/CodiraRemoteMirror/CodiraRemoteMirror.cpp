//===--- CodiraRemoteMirror.cpp - C wrapper for Reflection API -------------===//
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

#include "language/CodiraRemoteMirror/Platform.h"
#include "language/CodiraRemoteMirror/CodiraRemoteMirror.h"
#include <iostream>
#include <variant>

#define LANGUAGE_CLASS_IS_LANGUAGE_MASK language_reflection_classIsCodiraMask
extern "C" {
LANGUAGE_REMOTE_MIRROR_LINKAGE
unsigned long long language_reflection_classIsCodiraMask = 2;

LANGUAGE_REMOTE_MIRROR_LINKAGE uint32_t language_reflection_libraryVersion = 3;
}

#include "language/Demangling/Demangler.h"
#include "language/RemoteInspection/ReflectionContext.h"
#include "language/RemoteInspection/TypeLowering.h"
#include "language/Remote/CMemoryReader.h"
#include "language/Basic/Unreachable.h"

#if defined(__APPLE__) && defined(__MACH__)
#include <TargetConditionals.h>
#endif

using namespace language;
using namespace language::reflection;
using namespace language::remote;

using RuntimeWithObjCInterop =
    External<WithObjCInterop<RuntimeTarget<sizeof(uintptr_t)>>>;
using RuntimeNoObjCInterop =
    External<NoObjCInterop<RuntimeTarget<sizeof(uintptr_t)>>>;

using ReflectionContextWithObjCInterop =
    language::reflection::ReflectionContext<RuntimeWithObjCInterop>;
using ReflectionContextNoObjCInterop =
    language::reflection::ReflectionContext<RuntimeNoObjCInterop>;

struct CodiraReflectionContext {
  using ContextVariant =
      std::variant<std::unique_ptr<ReflectionContextWithObjCInterop>,
                   std::unique_ptr<ReflectionContextNoObjCInterop>>;

  ContextVariant context;
  std::vector<std::function<void()>> freeFuncs;
  std::vector<std::tuple<language_addr_t, language_addr_t>> dataSegments;

  std::function<void(void)> freeTemporaryAllocation = [] {};

  CodiraReflectionContext(bool objCInteropIsEnabled, MemoryReaderImpl impl) {
    auto Reader = std::make_shared<CMemoryReader>(impl);
    if (objCInteropIsEnabled) {
      context = std::make_unique<ReflectionContextWithObjCInterop>(Reader);
    } else {
      context = std::make_unique<ReflectionContextNoObjCInterop>(Reader);
    }
  }

  ~CodiraReflectionContext() {
    freeTemporaryAllocation();
    for (auto f : freeFuncs)
      f();
  }

  // Allocate a single temporary object that will stay allocated until the next
  // call to this method, or until the context is destroyed.
  template <typename T>
  T *allocateTemporaryObject() {
    freeTemporaryAllocation();
    T *obj = new T;
    freeTemporaryAllocation = [obj] { delete obj; };
    return obj;
  }

  // Allocate a single temporary object that will stay allocated until the next
  // call to allocateTemporaryObject, or until the context is destroyed. Does
  // NOT free any existing objects created with allocateTemporaryObject or
  // allocateSubsequentTemporaryObject. Use to allocate additional objects after
  // a call to allocateTemporaryObject when multiple objects are needed
  // simultaneously.
  template <typename T>
  T *allocateSubsequentTemporaryObject() {
    T *obj = new T;
    auto oldFree = freeTemporaryAllocation;
    freeTemporaryAllocation = [obj, oldFree] {
      delete obj;
      oldFree();
    };
    return obj;
  }

  // Call fn with a pointer to context.
  template <typename Fn>
  auto withContext(const Fn &fn) {
    return std::visit([&](auto &&context) { return fn(context.get()); },
                      this->context);
  }
};

uint16_t
language_reflection_getSupportedMetadataVersion() {
  return LANGUAGE_REFLECTION_METADATA_VERSION;
}

template <uint8_t WordSize>
static int minimalDataLayoutQueryFunction(void *ReaderContext,
                                          DataLayoutQueryType type,
                                          void *inBuffer, void *outBuffer) {
    // TODO: The following should be set based on the target.
    // This code sets it to match the platform this code was compiled for.
#if defined(__APPLE__) && __APPLE__
    auto applePlatform = true;
#else
    auto applePlatform = false;
#endif
#if defined(__APPLE__) && __APPLE__ && ((defined(TARGET_OS_IOS) && TARGET_OS_IOS) || (defined(TARGET_OS_IOS) && TARGET_OS_WATCH) || (defined(TARGET_OS_TV) && TARGET_OS_TV) || defined(__arm64__))
    auto iosDerivedPlatform = true;
#else
    auto iosDerivedPlatform = false;
#endif

  if (type == DLQ_GetPointerSize || type == DLQ_GetSizeSize) {
    auto result = static_cast<uint8_t *>(outBuffer);
    *result = WordSize;
    return 1;
  }
  if (type == DLQ_GetObjCReservedLowBits) {
    auto result = static_cast<uint8_t *>(outBuffer);
    if (applePlatform && !iosDerivedPlatform && WordSize == 8) {
      // Obj-C reserves low bit on 64-bit macOS only.
      // Other Apple platforms don't reserve this bit (even when
      // running on x86_64-based simulators).
      *result = 1;
    } else {
      *result = 0;
    }
    return 1;
  }
  if (type == DLQ_GetLeastValidPointerValue) {
    auto result = static_cast<uint64_t *>(outBuffer);
    if (applePlatform && WordSize == 8) {
      // Codira reserves the first 4GiB on all 64-bit Apple platforms
      *result = 0x100000000;
    } else {
      // Codira reserves the first 4KiB everywhere else
      *result = 0x1000;
    }
    return 1;
  }
  return 0;
}

// Caveat: This basically only works correctly if running on the same
// host as the target.  Otherwise, you'll need to use
// language_reflection_createReflectionContextWithDataLayout() below
// with an appropriate data layout query function that understands
// the target environment.
CodiraReflectionContextRef
language_reflection_createReflectionContext(void *ReaderContext,
                                         uint8_t PointerSize,
                                         FreeBytesFunction Free,
                                         ReadBytesFunction ReadBytes,
                                         GetStringLengthFunction GetStringLength,
                                         GetSymbolAddressFunction GetSymbolAddress) {
  assert((PointerSize == 4 || PointerSize == 8) && "We only support 32-bit and 64-bit.");
  assert(PointerSize == sizeof(uintptr_t) &&
         "We currently only support the pointer size this file was compiled with.");

  auto *DataLayout = PointerSize == 4 ? minimalDataLayoutQueryFunction<4>
                                      : minimalDataLayoutQueryFunction<8>;
  MemoryReaderImpl ReaderImpl {
    PointerSize,
    ReaderContext,
    DataLayout,
    Free,
    ReadBytes,
    GetStringLength,
    GetSymbolAddress
  };

  return new CodiraReflectionContext(LANGUAGE_OBJC_INTEROP, ReaderImpl);
}

CodiraReflectionContextRef
language_reflection_createReflectionContextWithDataLayout(void *ReaderContext,
                                    QueryDataLayoutFunction DataLayout,
                                    FreeBytesFunction Free,
                                    ReadBytesFunction ReadBytes,
                                    GetStringLengthFunction GetStringLength,
                                    GetSymbolAddressFunction GetSymbolAddress) {
  uint8_t PointerSize = sizeof(uintptr_t);
  MemoryReaderImpl ReaderImpl {
    PointerSize,
    ReaderContext,
    DataLayout,
    Free,
    ReadBytes,
    GetStringLength,
    GetSymbolAddress
  };

  // If the client implements DLQ_GetObjCInteropIsEnabled, use that value.
  // If they don't, use this platform's default.
  bool dataLayoutSaysObjCInteropIsEnabled = true;
  if (DataLayout(ReaderContext, DLQ_GetObjCInteropIsEnabled, nullptr,
                 (void *)&dataLayoutSaysObjCInteropIsEnabled)) {
    return new CodiraReflectionContext(dataLayoutSaysObjCInteropIsEnabled,
                                      ReaderImpl);
  } else {
    return new CodiraReflectionContext(LANGUAGE_OBJC_INTEROP, ReaderImpl);
  }
}

void language_reflection_destroyReflectionContext(CodiraReflectionContextRef ContextRef) {
  delete ContextRef;
}

template<typename Iterator>
ReflectionSection<Iterator> sectionFromInfo(const language_reflection_info_t &Info,
                              const language_reflection_section_pair_t &Section) {
  auto RemoteSectionStart = (uint64_t)(uintptr_t)Section.section.Begin
    - Info.LocalStartAddress
    + Info.RemoteStartAddress;

  auto Start = RemoteRef<void>(
      RemoteAddress(RemoteSectionStart, RemoteAddress::DefaultAddressSpace),
      Section.section.Begin);

  return ReflectionSection<Iterator>(Start,
             (uintptr_t)Section.section.End - (uintptr_t)Section.section.Begin);
}

template <typename Iterator>
ReflectionSection<Iterator> reflectionSectionFromLocalAndRemote(
    const language_reflection_section_mapping_t &Section) {
  auto RemoteSectionStart = (uint64_t)Section.remote_section.StartAddress;

  auto Start = RemoteRef<void>(
      RemoteAddress(RemoteSectionStart, RemoteAddress::DefaultAddressSpace),
      Section.local_section.Begin);

  return ReflectionSection<Iterator>(Start,
                                     (uintptr_t)Section.remote_section.Size);
}

void
language_reflection_addReflectionInfo(CodiraReflectionContextRef ContextRef,
                                   language_reflection_info_t Info) {
  ContextRef->withContext([&](auto *Context) {
    // The `offset` fields must be zero.
    if (Info.field.offset != 0
        || Info.associated_types.offset != 0
        || Info.builtin_types.offset != 0
        || Info.capture.offset != 0
        || Info.type_references.offset != 0
        || Info.reflection_strings.offset != 0) {
      std::cerr << "reserved field in language_reflection_info_t is not zero\n";
      abort();
    }

    ReflectionInfo ContextInfo{
        sectionFromInfo<FieldDescriptorIterator>(Info, Info.field),
        sectionFromInfo<AssociatedTypeIterator>(Info, Info.associated_types),
        sectionFromInfo<BuiltinTypeDescriptorIterator>(Info,
                                                       Info.builtin_types),
        sectionFromInfo<CaptureDescriptorIterator>(Info, Info.capture),
        sectionFromInfo<const void *>(Info, Info.type_references),
        sectionFromInfo<const void *>(Info, Info.reflection_strings),
        ReflectionSection<const void *>(nullptr, 0),
        ReflectionSection<MultiPayloadEnumDescriptorIterator>(0, 0),
        {}};

    Context->addReflectionInfo(ContextInfo);
  });
}

void language_reflection_addReflectionMappingInfo(
    CodiraReflectionContextRef ContextRef,
    language_reflection_mapping_info_t Info) {
  return ContextRef->withContext([&](auto *Context) {
    ReflectionInfo ContextInfo{
        reflectionSectionFromLocalAndRemote<FieldDescriptorIterator>(
            Info.field),
        reflectionSectionFromLocalAndRemote<AssociatedTypeIterator>(
            Info.associated_types),
        reflectionSectionFromLocalAndRemote<BuiltinTypeDescriptorIterator>(
            Info.builtin_types),
        reflectionSectionFromLocalAndRemote<CaptureDescriptorIterator>(
            Info.capture),
        reflectionSectionFromLocalAndRemote<const void *>(Info.type_references),
        reflectionSectionFromLocalAndRemote<const void *>(
            Info.reflection_strings),
        ReflectionSection<const void *>(nullptr, 0),
        MultiPayloadEnumSection(0, 0),
        {}};

    Context->addReflectionInfo(ContextInfo);
  });
}

int
language_reflection_addImage(CodiraReflectionContextRef ContextRef,
                          language_addr_t imageStart) {
  return ContextRef->withContext([&](auto *Context) {
    return Context
        ->addImage(
            RemoteAddress(imageStart, RemoteAddress::DefaultAddressSpace))
        .has_value();
  });
}

int
language_reflection_readIsaMask(CodiraReflectionContextRef ContextRef,
                             uintptr_t *outIsaMask) {
  return ContextRef->withContext([&](auto *Context) {
    auto isaMask = Context->readIsaMask();
    if (isaMask) {
      *outIsaMask = *isaMask;
      return true;
    }
    *outIsaMask = 0;
    return false;
  });
}

language_typeref_t
language_reflection_typeRefForMetadata(CodiraReflectionContextRef ContextRef,
                                    uintptr_t Metadata) {
  return ContextRef->withContext([&](auto *Context) {
    auto TR = Context->readTypeFromMetadata(
        RemoteAddress(Metadata, RemoteAddress::DefaultAddressSpace));
    return reinterpret_cast<language_typeref_t>(TR);
  });
}

int
language_reflection_ownsObject(CodiraReflectionContextRef ContextRef, uintptr_t Object) {
  return ContextRef->withContext([&](auto *Context) {
    return Context->ownsObject(
        RemoteAddress(Object, RemoteAddress::DefaultAddressSpace));
  });
}

int
language_reflection_ownsAddress(CodiraReflectionContextRef ContextRef, uintptr_t Address) {
  return ContextRef->withContext([&](auto *Context) {
    return Context->ownsAddress(
        RemoteAddress(Address, RemoteAddress::DefaultAddressSpace));
  });
}

int
language_reflection_ownsAddressStrict(CodiraReflectionContextRef ContextRef, uintptr_t Address) {
  return ContextRef->withContext([&](auto *Context) {
    return Context->ownsAddress(
        RemoteAddress(Address, RemoteAddress::DefaultAddressSpace), false);
  });
}

uintptr_t
language_reflection_metadataForObject(CodiraReflectionContextRef ContextRef,
                                   uintptr_t Object) {
  return ContextRef->withContext([&](auto *Context) -> uintptr_t {
    auto MetadataAddress = Context->readMetadataFromInstance(
        RemoteAddress(Object, RemoteAddress::DefaultAddressSpace));
    if (!MetadataAddress)
      return 0;
    return MetadataAddress->getRawAddress();
  });
}

language_reflection_ptr_t
language_reflection_metadataNominalTypeDescriptor(CodiraReflectionContextRef ContextRef,
                                               language_reflection_ptr_t MetadataAddress) {
  return ContextRef->withContext([&](auto *Context) {
    auto Address = Context->nominalTypeDescriptorFromMetadata(
        RemoteAddress(MetadataAddress, RemoteAddress::DefaultAddressSpace));
    return Address.getRawAddress();
  });
}

int language_reflection_metadataIsActor(CodiraReflectionContextRef ContextRef,
                                     language_reflection_ptr_t Metadata) {
  return ContextRef->withContext([&](auto *Context) {
    return Context->metadataIsActor(
        RemoteAddress(Metadata, RemoteAddress::DefaultAddressSpace));
  });
}

language_typeref_t
language_reflection_typeRefForInstance(CodiraReflectionContextRef ContextRef,
                                    uintptr_t Object) {
  return ContextRef->withContext([&](auto *Context) -> language_typeref_t {
    auto MetadataAddress = Context->readMetadataFromInstance(
        RemoteAddress(Object, RemoteAddress::DefaultAddressSpace));
    if (!MetadataAddress)
      return 0;
    auto TR = Context->readTypeFromMetadata(*MetadataAddress);
    return reinterpret_cast<language_typeref_t>(TR);
  });
}

language_typeref_t
language_reflection_typeRefForMangledTypeName(CodiraReflectionContextRef ContextRef,
                                           const char *MangledTypeName,
                                           uint64_t Length) {
  return ContextRef->withContext([&](auto *Context) {
    auto TR =
        Context->readTypeFromMangledName(MangledTypeName, Length).getType();
    return reinterpret_cast<language_typeref_t>(TR);
  });
}

char *
language_reflection_copyDemangledNameForTypeRef(
  CodiraReflectionContextRef ContextRef, language_typeref_t OpaqueTypeRef) {
  auto TR = reinterpret_cast<const TypeRef *>(OpaqueTypeRef);

  Demangle::Demangler Dem;
  auto Name = nodeToString(TR->getDemangling(Dem));
  return strdup(Name.c_str());
}

char *
language_reflection_copyNameForTypeRef(CodiraReflectionContextRef ContextRef,
                                    language_typeref_t OpaqueTypeRef,
                                    bool mangled) {
  auto TR = reinterpret_cast<const TypeRef *>(OpaqueTypeRef);

  Demangle::Demangler Dem;
  if (mangled) {
    auto Mangling = mangleNode(TR->getDemangling(Dem), Mangle::ManglingFlavor::Default);
    if (Mangling.isSuccess()) {
      return strdup(Mangling.result().c_str());
    }
  }
  else {
    auto Name = nodeToString(TR->getDemangling(Dem));
    return strdup(Name.c_str());
  }
  return nullptr;
}

LANGUAGE_REMOTE_MIRROR_LINKAGE
char *
language_reflection_copyDemangledNameForProtocolDescriptor(
  CodiraReflectionContextRef ContextRef, language_reflection_ptr_t Proto) {
  return ContextRef->withContext([&](auto *Context) {

    Demangle::Demangler Dem;
    auto Demangling = Context->readDemanglingForContextDescriptor(
        RemoteAddress(Proto, RemoteAddress::DefaultAddressSpace), Dem);
    auto Name = nodeToString(Demangling);
    return strdup(Name.c_str());
  });
}

language_typeref_t
language_reflection_genericArgumentOfTypeRef(language_typeref_t OpaqueTypeRef,
                                          unsigned Index) {
  auto TR = reinterpret_cast<const TypeRef *>(OpaqueTypeRef);

  if (auto BG = dyn_cast<BoundGenericTypeRef>(TR)) {
    auto &Params = BG->getGenericParams();
    assert(Index < Params.size());
    return reinterpret_cast<language_typeref_t>(Params[Index]);
  }
  return 0;
}

unsigned
language_reflection_genericArgumentCountOfTypeRef(language_typeref_t OpaqueTypeRef) {
  auto TR = reinterpret_cast<const TypeRef *>(OpaqueTypeRef);

  if (auto BG = dyn_cast<BoundGenericTypeRef>(TR)) {
    auto &Params = BG->getGenericParams();
    return Params.size();
  }
  return 0;
}

language_layout_kind_t getTypeInfoKind(const TypeInfo &TI) {
  switch (TI.getKind()) {
  case TypeInfoKind::Invalid: {
    return LANGUAGE_UNKNOWN;
  }
  case TypeInfoKind::Builtin: {
    auto &BuiltinTI = cast<BuiltinTypeInfo>(TI);
    if (BuiltinTI.getMangledTypeName() == "Bp")
      return LANGUAGE_RAW_POINTER;
    return LANGUAGE_BUILTIN;
  }
  case TypeInfoKind::Record: {
    auto &RecordTI = cast<RecordTypeInfo>(TI);
    switch (RecordTI.getRecordKind()) {
    case RecordKind::Invalid:
      return LANGUAGE_UNKNOWN;
    case RecordKind::Tuple:
      return LANGUAGE_TUPLE;
    case RecordKind::Struct:
      return LANGUAGE_STRUCT;
    case RecordKind::ThickFunction:
      return LANGUAGE_THICK_FUNCTION;
    case RecordKind::OpaqueExistential:
      return LANGUAGE_OPAQUE_EXISTENTIAL;
    case RecordKind::ClassExistential:
      return LANGUAGE_CLASS_EXISTENTIAL;
    case RecordKind::ErrorExistential:
      return LANGUAGE_ERROR_EXISTENTIAL;
    case RecordKind::ExistentialMetatype:
      return LANGUAGE_EXISTENTIAL_METATYPE;
    case RecordKind::ClassInstance:
      return LANGUAGE_CLASS_INSTANCE;
    case RecordKind::ClosureContext:
      return LANGUAGE_CLOSURE_CONTEXT;
    }
  }
  case TypeInfoKind::Enum: {
    auto &EnumTI = cast<EnumTypeInfo>(TI);
    switch (EnumTI.getEnumKind()) {
    case EnumKind::NoPayloadEnum:
      return LANGUAGE_NO_PAYLOAD_ENUM;
    case EnumKind::SinglePayloadEnum:
      return LANGUAGE_SINGLE_PAYLOAD_ENUM;
    case EnumKind::MultiPayloadEnum:
      return LANGUAGE_MULTI_PAYLOAD_ENUM;
    }
  }
  case TypeInfoKind::Reference: {
    auto &ReferenceTI = cast<ReferenceTypeInfo>(TI);
    switch (ReferenceTI.getReferenceKind()) {
    case ReferenceKind::Strong: return LANGUAGE_STRONG_REFERENCE;
#define REF_STORAGE(Name, name, NAME) \
    case ReferenceKind::Name: return LANGUAGE_##NAME##_REFERENCE;
#include "language/AST/ReferenceStorage.def"
    }
  }

  case TypeInfoKind::Array: {
    return LANGUAGE_ARRAY;
  }
  }

  language_unreachable("Unhandled TypeInfoKind in switch");
}

static language_typeinfo_t convertTypeInfo(const TypeInfo *TI) {
  if (TI == nullptr) {
    return {
      LANGUAGE_UNKNOWN,
      0,
      0,
      0,
      0
    };
  }

  unsigned NumFields = 0;
  if (auto *RecordTI = dyn_cast<EnumTypeInfo>(TI)) {
    NumFields = RecordTI->getNumCases();
  } else if (auto *RecordTI = dyn_cast<RecordTypeInfo>(TI)) {
    NumFields = RecordTI->getNumFields();
  }

  return {
    getTypeInfoKind(*TI),
    TI->getSize(),
    TI->getAlignment(),
    TI->getStride(),
    NumFields
  };
}

static language_childinfo_t convertChild(const TypeInfo *TI, unsigned Index) {
  if (!TI)
    return {};

  const FieldInfo *FieldInfo = nullptr;
  if (auto *EnumTI = dyn_cast<EnumTypeInfo>(TI)) {
    FieldInfo = &(EnumTI->getCases()[Index]);
  } else if (auto *RecordTI = dyn_cast<RecordTypeInfo>(TI)) {
    FieldInfo = &(RecordTI->getFields()[Index]);
  } else {
    assert(false && "convertChild(TI): TI must be record or enum typeinfo");
    return {
      "unknown TypeInfo kind",
      0,
      LANGUAGE_UNKNOWN,
      0,
    };
  }

  return {
    FieldInfo->Name.c_str(),
    FieldInfo->Offset,
    getTypeInfoKind(FieldInfo->TI),
    reinterpret_cast<language_typeref_t>(FieldInfo->TR),
  };
}

template <typename ReflectionContext>
static language_layout_kind_t convertAllocationChunkKind(
    typename ReflectionContext::AsyncTaskAllocationChunk::ChunkKind Kind) {
  switch (Kind) {
  case ReflectionContext::AsyncTaskAllocationChunk::ChunkKind::Unknown:
    return LANGUAGE_UNKNOWN;
  case ReflectionContext::AsyncTaskAllocationChunk::ChunkKind::NonPointer:
    return LANGUAGE_BUILTIN;
  case ReflectionContext::AsyncTaskAllocationChunk::ChunkKind::RawPointer:
    return LANGUAGE_RAW_POINTER;
  case ReflectionContext::AsyncTaskAllocationChunk::ChunkKind::StrongReference:
    return LANGUAGE_STRONG_REFERENCE;
  case ReflectionContext::AsyncTaskAllocationChunk::ChunkKind::UnownedReference:
    return LANGUAGE_UNOWNED_REFERENCE;
  case ReflectionContext::AsyncTaskAllocationChunk::ChunkKind::WeakReference:
    return LANGUAGE_WEAK_REFERENCE;
  case ReflectionContext::AsyncTaskAllocationChunk::ChunkKind::
      UnmanagedReference:
    return LANGUAGE_UNMANAGED_REFERENCE;
  }
}

static const char *returnableCString(CodiraReflectionContextRef ContextRef,
                                     std::optional<std::string> String) {
  if (String) {
    auto *TmpStr = ContextRef->allocateTemporaryObject<std::string>();
    *TmpStr = *String;
    return TmpStr->c_str();
  }
  return nullptr;
}

language_typeinfo_t
language_reflection_infoForTypeRef(CodiraReflectionContextRef ContextRef,
                                language_typeref_t OpaqueTypeRef) {
  return ContextRef->withContext([&](auto *Context) {
    auto TR = reinterpret_cast<const TypeRef *>(OpaqueTypeRef);
    auto TI = Context->getTypeInfo(TR, nullptr);
    return convertTypeInfo(TI);
  });
}

language_childinfo_t
language_reflection_childOfTypeRef(CodiraReflectionContextRef ContextRef,
                                language_typeref_t OpaqueTypeRef,
                                unsigned Index) {
  return ContextRef->withContext([&](auto *Context) {
    auto TR = reinterpret_cast<const TypeRef *>(OpaqueTypeRef);
    auto *TI = Context->getTypeInfo(TR, nullptr);
    return convertChild(TI, Index);
  });
}

language_typeinfo_t
language_reflection_infoForMetadata(CodiraReflectionContextRef ContextRef,
                                 uintptr_t Metadata) {
  return ContextRef->withContext([&](auto *Context) {
    auto *TI = Context->getMetadataTypeInfo(
        RemoteAddress(Metadata, RemoteAddress::DefaultAddressSpace), nullptr);
    return convertTypeInfo(TI);
  });
}

language_childinfo_t
language_reflection_childOfMetadata(CodiraReflectionContextRef ContextRef,
                                 uintptr_t Metadata,
                                 unsigned Index) {
  return ContextRef->withContext([&](auto *Context) {
    auto *TI = Context->getMetadataTypeInfo(
        RemoteAddress(Metadata, RemoteAddress::DefaultAddressSpace), nullptr);
    return convertChild(TI, Index);
  });
}

language_typeinfo_t
language_reflection_infoForInstance(CodiraReflectionContextRef ContextRef,
                                 uintptr_t Object) {
  return ContextRef->withContext([&](auto *Context) {
    auto *TI = Context->getInstanceTypeInfo(
        RemoteAddress(Object, RemoteAddress::DefaultAddressSpace), nullptr);
    return convertTypeInfo(TI);
  });
}

language_childinfo_t
language_reflection_childOfInstance(CodiraReflectionContextRef ContextRef,
                                 uintptr_t Object,
                                 unsigned Index) {
  return ContextRef->withContext([&](auto *Context) {
    auto *TI = Context->getInstanceTypeInfo(
        RemoteAddress(Object, RemoteAddress::DefaultAddressSpace), nullptr);
    return convertChild(TI, Index);
  });
}

int language_reflection_projectExistential(CodiraReflectionContextRef ContextRef,
                                        language_addr_t ExistentialAddress,
                                        language_typeref_t ExistentialTypeRef,
                                        language_typeref_t *InstanceTypeRef,
                                        language_addr_t *StartOfInstanceData) {
  return ContextRef->withContext([&](auto *Context) {
    auto ExistentialTR = reinterpret_cast<const TypeRef *>(ExistentialTypeRef);
    auto RemoteExistentialAddress =
        RemoteAddress(ExistentialAddress, RemoteAddress::DefaultAddressSpace);
    const TypeRef *InstanceTR = nullptr;
    RemoteAddress RemoteStartOfInstanceData;
    auto Success = Context->projectExistential(
        RemoteExistentialAddress, ExistentialTR, &InstanceTR,
        &RemoteStartOfInstanceData, nullptr);

    if (Success) {
      *InstanceTypeRef = reinterpret_cast<language_typeref_t>(InstanceTR);
      *StartOfInstanceData = RemoteStartOfInstanceData.getRawAddress();
    }

    return Success;
  });
}

int language_reflection_projectExistentialAndUnwrapClass(CodiraReflectionContextRef ContextRef,
                                        language_addr_t ExistentialAddress,
                                        language_typeref_t ExistentialTypeRef,
                                        language_typeref_t *InstanceTypeRef,
                                        language_addr_t *StartOfInstanceData) {
  return ContextRef->withContext([&](auto *Context) {
    auto ExistentialTR = reinterpret_cast<const TypeRef *>(ExistentialTypeRef);
    auto RemoteExistentialAddress =
        RemoteAddress(ExistentialAddress, RemoteAddress::DefaultAddressSpace);
    auto Pair = Context->projectExistentialAndUnwrapClass(
        RemoteExistentialAddress, *ExistentialTR);
    if (!Pair.has_value())
      return false;
    *InstanceTypeRef =
        reinterpret_cast<language_typeref_t>(std::get<const TypeRef *>(*Pair));
    *StartOfInstanceData = std::get<RemoteAddress>(*Pair).getRawAddress();

    return true;
  });
}

int language_reflection_projectEnumValue(CodiraReflectionContextRef ContextRef,
                                      language_addr_t EnumAddress,
                                      language_typeref_t EnumTypeRef,
                                      int *CaseIndex) {
  return ContextRef->withContext([&](auto *Context) {
    auto EnumTR = reinterpret_cast<const TypeRef *>(EnumTypeRef);
    auto RemoteEnumAddress =
        RemoteAddress(EnumAddress, RemoteAddress::DefaultAddressSpace);
    if (!Context->projectEnumValue(RemoteEnumAddress, EnumTR, CaseIndex,
                                   nullptr)) {
      return false;
    }
    auto TI = Context->getTypeInfo(EnumTR, nullptr);
    auto *RecordTI = dyn_cast<EnumTypeInfo>(TI);
    assert(RecordTI != nullptr);
    if (static_cast<size_t>(*CaseIndex) >= RecordTI->getNumCases()) {
      return false;
    }
    return true;
  });
}

void language_reflection_dumpTypeRef(language_typeref_t OpaqueTypeRef) {
  auto TR = reinterpret_cast<const TypeRef *>(OpaqueTypeRef);
  if (TR == nullptr) {
    std::cout << "<null type reference>\n";
  } else {
    TR->dump(std::cout);
  }
}

void language_reflection_dumpInfoForTypeRef(CodiraReflectionContextRef ContextRef,
                                         language_typeref_t OpaqueTypeRef) {
  ContextRef->withContext([&](auto *Context) {
    auto TR = reinterpret_cast<const TypeRef *>(OpaqueTypeRef);
    auto TI = Context->getTypeInfo(TR, nullptr);
    if (TI == nullptr) {
      std::cout << "<null type info>\n";
    } else {
      TI->dump(std::cout);
      Demangle::Demangler Dem;
      auto Mangling = mangleNode(TR->getDemangling(Dem), Mangle::ManglingFlavor::Default);
      std::string MangledName;
      if (Mangling.isSuccess()) {
        MangledName = Mangling.result();
        std::cout << "Mangled name: " << MANGLING_PREFIX_STR << MangledName
                  << "\n";
      } else {
        MangledName = "<failed to mangle name>";
        std::cout 
          << "Failed to get mangled name: Node " << Mangling.error().node
          << " error " << Mangling.error().code << ":"
          << Mangling.error().line << "\n";
      }

      char *DemangledName =
          language_reflection_copyNameForTypeRef(ContextRef, OpaqueTypeRef, false);
      std::cout << "Demangled name: " << DemangledName << "\n";
      free(DemangledName);
    }
  });
}

void language_reflection_dumpInfoForMetadata(CodiraReflectionContextRef ContextRef,
                                          uintptr_t Metadata) {
  ContextRef->withContext([&](auto *Context) {
    auto TI = Context->getMetadataTypeInfo(
        RemoteAddress(Metadata, RemoteAddress::DefaultAddressSpace), nullptr);
    if (TI == nullptr) {
      std::cout << "<null type info>\n";
    } else {
      TI->dump(std::cout);
    }
  });
}

void language_reflection_dumpInfoForInstance(CodiraReflectionContextRef ContextRef,
                                          uintptr_t Object) {
  ContextRef->withContext([&](auto *Context) {
    auto TI = Context->getInstanceTypeInfo(
        RemoteAddress(Object, RemoteAddress::DefaultAddressSpace), nullptr);
    if (TI == nullptr) {
      std::cout << "<null type info>\n";
    } else {
      TI->dump(std::cout);
    }
  });
}

size_t language_reflection_demangle(const char *MangledName, size_t Length,
                                 char *OutDemangledName, size_t MaxLength) {
  if (MangledName == nullptr || Length == 0)
    return 0;

  std::string Mangled(MangledName, Length);
  auto Demangled = Demangle::demangleTypeAsString(Mangled);
  strncpy(OutDemangledName, Demangled.c_str(), MaxLength);
  return Demangled.size();
}

const char *language_reflection_iterateConformanceCache(
  CodiraReflectionContextRef ContextRef,
  void (*Call)(language_reflection_ptr_t Type,
               language_reflection_ptr_t Proto,
               void *ContextPtr),
  void *ContextPtr) {
  return ContextRef->withContext([&](auto *Context) {
    auto Error = Context->iterateConformances([&](auto Type, auto Proto) {
      Call(Type.getRawAddress(), Proto.getRawAddress(), ContextPtr);
    });
    return returnableCString(ContextRef, Error);
  });
}

const char *language_reflection_iterateMetadataAllocations(
  CodiraReflectionContextRef ContextRef,
  void (*Call)(language_metadata_allocation_t Allocation,
               void *ContextPtr),
  void *ContextPtr) {
  return ContextRef->withContext([&](auto *Context) {
    auto Error = Context->iterateMetadataAllocations([&](auto Allocation) {
      language_metadata_allocation CAllocation;
      CAllocation.Tag = Allocation.Tag;
      CAllocation.Ptr = Allocation.Ptr;
      CAllocation.Size = Allocation.Size;
      Call(CAllocation, ContextPtr);
    });
    return returnableCString(ContextRef, Error);
  });
}

// Convert Allocation to a MetadataAllocation<Runtime>, where <Runtime> is
// the same as the <Runtime> template of Context.
//
// Accepting the Context parameter is a workaround for templated lambda callers
// not having direct access to <Runtime>. The Codira project doesn't compile
// with a new enough C++ version to use explicitly-templated lambdas, so we
// need some other method of extracting <Runtime>.
template <typename Runtime>
static MetadataAllocation<Runtime> convertMetadataAllocation(
    const language::reflection::ReflectionContext<Runtime> *Context,
    const language_metadata_allocation_t &Allocation) {
  (void)Context;

  MetadataAllocation<Runtime> ConvertedAllocation;
  ConvertedAllocation.Tag = Allocation.Tag;
  ConvertedAllocation.Ptr = Allocation.Ptr;
  ConvertedAllocation.Size = Allocation.Size;
  return ConvertedAllocation;
}

language_reflection_ptr_t language_reflection_allocationMetadataPointer(
  CodiraReflectionContextRef ContextRef,
  language_metadata_allocation_t Allocation) {
  return ContextRef->withContext([&](auto *Context) {
    auto ConvertedAllocation = convertMetadataAllocation(Context, Allocation);
    return Context->allocationMetadataPointer(ConvertedAllocation);
  });
}

const char *language_reflection_metadataAllocationTagName(
    CodiraReflectionContextRef ContextRef, language_metadata_allocation_tag_t Tag) {
  return ContextRef->withContext([&](auto *Context) {
    auto Result = Context->metadataAllocationTagName(Tag);
    return returnableCString(ContextRef, Result);
  });
}

int language_reflection_metadataAllocationCacheNode(
    CodiraReflectionContextRef ContextRef,
    language_metadata_allocation_t Allocation,
    language_metadata_cache_node_t *OutNode) {
  return ContextRef->withContext([&](auto *Context) {
    auto ConvertedAllocation = convertMetadataAllocation(Context, Allocation);

    auto Result = Context->metadataAllocationCacheNode(ConvertedAllocation);
    if (!Result)
      return 0;

    OutNode->Left = Result->Left;
    OutNode->Right = Result->Right;
    return 1;
  });
}

const char *language_reflection_iterateMetadataAllocationBacktraces(
    CodiraReflectionContextRef ContextRef,
    language_metadataAllocationBacktraceIterator Call, void *ContextPtr) {
  return ContextRef->withContext([&](auto *Context) {
    auto Error = Context->iterateMetadataAllocationBacktraces(
        [&](auto AllocationPtr, auto Count, auto Ptrs) {
          // Ptrs is an array of StoredPointer, but the callback expects an
          // array of language_reflection_ptr_t. Those may are not always the same
          // type. (For example, language_reflection_ptr_t can be 64-bit on 32-bit
          // systems, while StoredPointer is always the pointer size of the
          // target system.) Convert the array to an array of
          // language_reflection_ptr_t.
          std::vector<language_reflection_ptr_t> ConvertedPtrs{&Ptrs[0],
                                                            &Ptrs[Count]};
          Call(AllocationPtr, Count, ConvertedPtrs.data(), ContextPtr);
        });
    return returnableCString(ContextRef, Error);
  });
}

language_async_task_slab_return_t
language_reflection_asyncTaskSlabPointer(CodiraReflectionContextRef ContextRef,
                                      language_reflection_ptr_t AsyncTaskPtr) {
  return ContextRef->withContext([&](auto *Context) {
    // We only care about the AllocatorSlabPtr field. Disable child task and
    // async backtrace iteration to save wasted work.
    unsigned ChildTaskLimit = 0;
    unsigned AsyncBacktraceLimit = 0;

    auto [Error, TaskInfo] = Context->asyncTaskInfo(
        RemoteAddress(AsyncTaskPtr, RemoteAddress::DefaultAddressSpace),
        ChildTaskLimit, AsyncBacktraceLimit);

    language_async_task_slab_return_t Result = {};
    if (Error) {
      Result.Error = returnableCString(ContextRef, Error);
    }
    Result.SlabPtr = TaskInfo.AllocatorSlabPtr;
    return Result;
  });
}

language_async_task_slab_allocations_return_t
language_reflection_asyncTaskSlabAllocations(CodiraReflectionContextRef ContextRef,
                                          language_reflection_ptr_t SlabPtr) {
  return ContextRef->withContext([&](auto *Context) {
    auto [Error, Info] = Context->asyncTaskSlabAllocations(SlabPtr);

    language_async_task_slab_allocations_return_t Result = {};
    if (Result.Error) {
      Result.Error = returnableCString(ContextRef, Error);
      return Result;
    }

    Result.NextSlab = Info.NextSlab;
    Result.SlabSize = Info.SlabSize;

    auto *Chunks = ContextRef->allocateTemporaryObject<
        std::vector<language_async_task_allocation_chunk_t>>();
    Chunks->reserve(Info.Chunks.size());
    for (auto &Chunk : Info.Chunks) {
      language_async_task_allocation_chunk_t ConvertedChunk;
      ConvertedChunk.Start = Chunk.Start;
      ConvertedChunk.Length = Chunk.Length;

      // This pedantry is required to properly template over *Context.
      ConvertedChunk.Kind = convertAllocationChunkKind<
          typename std::pointer_traits<decltype(Context)>::element_type>(
          Chunk.Kind);

      Chunks->push_back(ConvertedChunk);
    }

    Result.ChunkCount = Chunks->size();
    Result.Chunks = Chunks->data();

    return Result;
  });
}

language_async_task_info_t
language_reflection_asyncTaskInfo(CodiraReflectionContextRef ContextRef,
                               language_reflection_ptr_t AsyncTaskPtr) {
  return ContextRef->withContext([&](auto *Context) {
    // Limit the child task and async backtrace iteration to semi-reasonable
    // numbers to avoid doing excessive work on bad data.
    unsigned ChildTaskLimit = 1000000;
    unsigned AsyncBacktraceLimit = 1000;

    auto [Error, TaskInfo] = Context->asyncTaskInfo(
        RemoteAddress(AsyncTaskPtr, RemoteAddress::DefaultAddressSpace),
        ChildTaskLimit, AsyncBacktraceLimit);

    language_async_task_info_t Result = {};
    if (Error) {
      Result.Error = returnableCString(ContextRef, Error);
      return Result;
    }

    Result.Kind = TaskInfo.Kind;
    Result.EnqueuePriority = TaskInfo.EnqueuePriority;
    Result.IsChildTask = TaskInfo.IsChildTask;
    Result.IsFuture = TaskInfo.IsFuture;
    Result.IsGroupChildTask = TaskInfo.IsGroupChildTask;
    Result.IsAsyncLetTask = TaskInfo.IsAsyncLetTask;
    Result.IsSynchronousStartTask = TaskInfo.IsSynchronousStartTask;

    Result.MaxPriority = TaskInfo.MaxPriority;
    Result.IsCancelled = TaskInfo.IsCancelled;
    Result.IsStatusRecordLocked = TaskInfo.IsStatusRecordLocked;
    Result.IsEscalated = TaskInfo.IsEscalated;
    Result.HasIsRunning = TaskInfo.HasIsRunning;
    Result.IsRunning = TaskInfo.IsRunning;
    Result.IsEnqueued = TaskInfo.IsEnqueued;
    Result.Id = TaskInfo.Id;

    Result.HasThreadPort = TaskInfo.HasThreadPort;
    Result.ThreadPort = TaskInfo.ThreadPort;

    Result.RunJob = TaskInfo.RunJob;
    Result.AllocatorSlabPtr = TaskInfo.AllocatorSlabPtr;

    auto *ChildTasks =
        ContextRef
            ->allocateTemporaryObject<std::vector<language_reflection_ptr_t>>();
    std::copy(TaskInfo.ChildTasks.begin(), TaskInfo.ChildTasks.end(),
              std::back_inserter(*ChildTasks));
    Result.ChildTaskCount = ChildTasks->size();
    Result.ChildTasks = ChildTasks->data();

    auto *AsyncBacktraceFrames = ContextRef->allocateSubsequentTemporaryObject<
        std::vector<language_reflection_ptr_t>>();
    std::copy(TaskInfo.AsyncBacktraceFrames.begin(),
              TaskInfo.AsyncBacktraceFrames.end(),
              std::back_inserter(*AsyncBacktraceFrames));
    Result.AsyncBacktraceFramesCount = AsyncBacktraceFrames->size();
    Result.AsyncBacktraceFrames = AsyncBacktraceFrames->data();

    return Result;
  });
}

language_actor_info_t
language_reflection_actorInfo(CodiraReflectionContextRef ContextRef,
                           language_reflection_ptr_t ActorPtr) {
  return ContextRef->withContext([&](auto *Context) {
    auto [Error, ActorInfo] = Context->actorInfo(
        RemoteAddress(ActorPtr, RemoteAddress::DefaultAddressSpace));

    language_actor_info_t Result = {};
    Result.Error = returnableCString(ContextRef, Error);
    Result.State = ActorInfo.State;
    Result.IsDistributedRemote = ActorInfo.IsDistributedRemote;
    Result.IsPriorityEscalated = ActorInfo.IsPriorityEscalated;
    Result.MaxPriority = ActorInfo.MaxPriority;
    Result.FirstJob = ActorInfo.FirstJob;

    Result.HasThreadPort = ActorInfo.HasThreadPort;
    Result.ThreadPort = ActorInfo.ThreadPort;

    return Result;
  });
}

language_reflection_ptr_t
language_reflection_nextJob(CodiraReflectionContextRef ContextRef,
                         language_reflection_ptr_t JobPtr) {
  return ContextRef->withContext([&](auto *Context) {
    return Context->nextJob(
        RemoteAddress(JobPtr, RemoteAddress::DefaultAddressSpace));
  });
}
