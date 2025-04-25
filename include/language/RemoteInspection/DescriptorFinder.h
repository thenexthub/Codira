//===--------------- DescriptorFinder.h -------------------------*- C++ -*-===//
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

#ifndef SWIFT_REFLECTION_DESCRIPTOR_FINDER_H
#define SWIFT_REFLECTION_DESCRIPTOR_FINDER_H

#include "language/Demangling/Demangle.h"
#include "language/RemoteInspection/Records.h"
#include "llvm/ADT/StringRef.h"

namespace language {
namespace reflection {

class TypeRef;

/// An abstract interface for a builtin type descriptor.
struct BuiltinTypeDescriptorBase {
  const uint32_t Size;
  const uint32_t Alignment;
  const uint32_t Stride;
  const uint32_t NumExtraInhabitants;
  const bool IsBitwiseTakable;

  BuiltinTypeDescriptorBase(uint32_t Size, uint32_t Alignment, uint32_t Stride,
                            uint32_t NumExtraInhabitants, bool IsBitwiseTakable)
      : Size(Size), Alignment(Alignment), Stride(Stride),
        NumExtraInhabitants(NumExtraInhabitants),
        IsBitwiseTakable(IsBitwiseTakable) {}

  virtual ~BuiltinTypeDescriptorBase(){};

  virtual llvm::StringRef getMangledTypeName() = 0;
};

/// An abstract interface for a field record.
struct FieldRecordBase {
  const bool IsIndirectCase;
  const bool IsVar;
  const bool HasMangledTypeName;

  FieldRecordBase(bool IsIndirectCase, bool IsVar,
                       bool HasMangledTypeName)
      : IsIndirectCase(IsIndirectCase), IsVar(IsVar),
        HasMangledTypeName(HasMangledTypeName) {}

  virtual ~FieldRecordBase(){};

  virtual llvm::StringRef getFieldName() = 0;
  virtual Demangle::Node *getDemangledTypeName() = 0;
};

/// An abstract interface for a field descriptor.
struct FieldDescriptorBase {
  const FieldDescriptorKind Kind;
  const bool HasSuperClass;

  FieldDescriptorBase(FieldDescriptorKind Kind, bool HasSuperClass)
      : Kind(Kind), HasSuperClass(HasSuperClass) {}

  bool isEnum() const {
    return (Kind == FieldDescriptorKind::Enum ||
            Kind == FieldDescriptorKind::MultiPayloadEnum);
  }

  bool isClass() const {
    return (Kind == FieldDescriptorKind::Class ||
            Kind == FieldDescriptorKind::ObjCClass);
  }

  bool isProtocol() const {
    return (Kind == FieldDescriptorKind::Protocol ||
            Kind == FieldDescriptorKind::ClassProtocol ||
            Kind == FieldDescriptorKind::ObjCProtocol);
  }

  bool isStruct() const {
    return Kind == FieldDescriptorKind::Struct;
  }

  virtual ~FieldDescriptorBase(){};

  virtual Demangle::Node *demangleSuperclass() = 0;
  virtual std::vector<std::unique_ptr<FieldRecordBase>>
  getFieldRecords() = 0;
};

struct MultiPayloadEnumDescriptorBase {
  virtual ~MultiPayloadEnumDescriptorBase(){};

  virtual llvm::StringRef getMangledTypeName() = 0;

  virtual uint32_t getContentsSizeInWords() const = 0;

  virtual size_t getSizeInBytes() const = 0;

  virtual uint32_t getFlags() const = 0;

  virtual bool usesPayloadSpareBits() const = 0;

  virtual uint32_t getPayloadSpareBitMaskByteOffset() const = 0;

  virtual uint32_t getPayloadSpareBitMaskByteCount() const = 0;

  virtual const uint8_t *getPayloadSpareBits() const = 0;

};
/// Interface for finding type descriptors. Implementors may provide descriptors
/// that live inside or outside reflection metadata.
struct DescriptorFinder {
  virtual ~DescriptorFinder(){};

  virtual std::unique_ptr<BuiltinTypeDescriptorBase>
  getBuiltinTypeDescriptor(const TypeRef *TR) = 0;


  virtual std::unique_ptr<FieldDescriptorBase>
  getFieldDescriptor(const TypeRef *TR) = 0;

  virtual std::unique_ptr<MultiPayloadEnumDescriptorBase>
  getMultiPayloadEnumDescriptor(const TypeRef *TR) { abort(); };
};

} // namespace reflection
} // namespace language
#endif
