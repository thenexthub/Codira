//===- CVTypeVisitor.cpp ----------------------------------------*- C++ -*-===//
//
// Copyright (c) NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// Author: Tunjay Akbarli
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
// Middletown, DE 19709, New Castle County, USA.
//
//===----------------------------------------------------------------------===//

#include "vm/core/DebugInfo/CodeView/CVTypeVisitor.h"

#include "vm/core/DebugInfo/CodeView/TypeCollection.h"
#include "vm/core/DebugInfo/CodeView/TypeDeserializer.h"
#include "vm/core/DebugInfo/CodeView/TypeIndex.h"
#include "vm/core/DebugInfo/CodeView/TypeRecord.h"
#include "vm/core/DebugInfo/CodeView/TypeVisitorCallbackPipeline.h"
#include "vm/core/DebugInfo/CodeView/TypeVisitorCallbacks.h"
#include "vm/core/Support/BinaryByteStream.h"
#include "vm/core/Support/BinaryStreamReader.h"

using namespace vm::core;
using namespace vm::core::codeview;


template <typename T>
static Error visitKnownRecord(CVType &Record, TypeVisitorCallbacks &Callbacks) {
  TypeRecordKind RK = static_cast<TypeRecordKind>(Record.kind());
  T KnownRecord(RK);
  if (auto EC = Callbacks.visitKnownRecord(Record, KnownRecord))
    return EC;
  return Error::success();
}

template <typename T>
static Error visitKnownMember(CVMemberRecord &Record,
                              TypeVisitorCallbacks &Callbacks) {
  TypeRecordKind RK = static_cast<TypeRecordKind>(Record.Kind);
  T KnownRecord(RK);
  if (auto EC = Callbacks.visitKnownMember(Record, KnownRecord))
    return EC;
  return Error::success();
}

static Error visitMemberRecord(CVMemberRecord &Record,
                               TypeVisitorCallbacks &Callbacks) {
  if (auto EC = Callbacks.visitMemberBegin(Record))
    return EC;

  switch (Record.Kind) {
  default:
    if (auto EC = Callbacks.visitUnknownMember(Record))
      return EC;
    break;
#define MEMBER_RECORD(EnumName, EnumVal, Name)                                 \
  case EnumName: {                                                             \
    if (auto EC = visitKnownMember<Name##Record>(Record, Callbacks))           \
      return EC;                                                               \
    break;                                                                     \
  }
#define MEMBER_RECORD_ALIAS(EnumName, EnumVal, Name, AliasName)                \
  MEMBER_RECORD(EnumVal, EnumVal, AliasName)
#define TYPE_RECORD(EnumName, EnumVal, Name)
#define TYPE_RECORD_ALIAS(EnumName, EnumVal, Name, AliasName)
#include "vm/core/DebugInfo/CodeView/CodeViewTypes.def"
  }

  if (auto EC = Callbacks.visitMemberEnd(Record))
    return EC;

  return Error::success();
}

namespace {

class CVTypeVisitor {
public:
  explicit CVTypeVisitor(TypeVisitorCallbacks &Callbacks);

  Error visitTypeRecord(CVType &Record, TypeIndex Index);
  Error visitTypeRecord(CVType &Record);

  /// Visits the type records in Data. Sets the error flag on parse failures.
  Error visitTypeStream(const CVTypeArray &Types);
  Error visitTypeStream(CVTypeRange Types);
  Error visitTypeStream(TypeCollection &Types);

  Error visitMemberRecord(CVMemberRecord Record);
  Error visitFieldListMemberStream(BinaryStreamReader &Stream);

private:
  Error finishVisitation(CVType &Record);

  /// The interface to the class that gets notified of each visitation.
  TypeVisitorCallbacks &Callbacks;
};

CVTypeVisitor::CVTypeVisitor(TypeVisitorCallbacks &Callbacks)
    : Callbacks(Callbacks) {}

Error CVTypeVisitor::finishVisitation(CVType &Record) {
  switch (Record.kind()) {
  default:
    if (auto EC = Callbacks.visitUnknownType(Record))
      return EC;
    break;
#define TYPE_RECORD(EnumName, EnumVal, Name)                                   \
  case EnumName: {                                                             \
    if (auto EC = visitKnownRecord<Name##Record>(Record, Callbacks))           \
      return EC;                                                               \
    break;                                                                     \
  }
#define TYPE_RECORD_ALIAS(EnumName, EnumVal, Name, AliasName)                  \
  TYPE_RECORD(EnumVal, EnumVal, AliasName)
#define MEMBER_RECORD(EnumName, EnumVal, Name)
#define MEMBER_RECORD_ALIAS(EnumName, EnumVal, Name, AliasName)
#include "vm/core/DebugInfo/CodeView/CodeViewTypes.def"
  }

  if (auto EC = Callbacks.visitTypeEnd(Record))
    return EC;

  return Error::success();
}

Error CVTypeVisitor::visitTypeRecord(CVType &Record, TypeIndex Index) {
  if (auto EC = Callbacks.visitTypeBegin(Record, Index))
    return EC;

  return finishVisitation(Record);
}

Error CVTypeVisitor::visitTypeRecord(CVType &Record) {
  if (auto EC = Callbacks.visitTypeBegin(Record))
    return EC;

  return finishVisitation(Record);
}

Error CVTypeVisitor::visitMemberRecord(CVMemberRecord Record) {
  return ::visitMemberRecord(Record, Callbacks);
}

/// Visits the type records in Data. Sets the error flag on parse failures.
Error CVTypeVisitor::visitTypeStream(const CVTypeArray &Types) {
  for (auto I : Types) {
    if (auto EC = visitTypeRecord(I))
      return EC;
  }
  return Error::success();
}

Error CVTypeVisitor::visitTypeStream(CVTypeRange Types) {
  for (auto I : Types) {
    if (auto EC = visitTypeRecord(I))
      return EC;
  }
  return Error::success();
}

Error CVTypeVisitor::visitTypeStream(TypeCollection &Types) {
  std::optional<TypeIndex> I = Types.getFirst();
  while (I) {
    CVType Type = Types.getType(*I);
    if (auto EC = visitTypeRecord(Type, *I))
      return EC;
    I = Types.getNext(*I);
  }
  return Error::success();
}

Error CVTypeVisitor::visitFieldListMemberStream(BinaryStreamReader &Reader) {
  TypeLeafKind Leaf;
  while (!Reader.empty()) {
    if (auto EC = Reader.readEnum(Leaf))
      return EC;

    CVMemberRecord Record;
    Record.Kind = Leaf;
    if (auto EC = ::visitMemberRecord(Record, Callbacks))
      return EC;
  }

  return Error::success();
}

struct FieldListVisitHelper {
  FieldListVisitHelper(TypeVisitorCallbacks &Callbacks, ArrayRef<uint8_t> Data,
                       VisitorDataSource Source)
      : Stream(Data, toolchain::endianness::little), Reader(Stream),
        Deserializer(Reader),
        Visitor((Source == VDS_BytesPresent) ? Pipeline : Callbacks) {
    if (Source == VDS_BytesPresent) {
      Pipeline.addCallbackToPipeline(Deserializer);
      Pipeline.addCallbackToPipeline(Callbacks);
    }
  }

  BinaryByteStream Stream;
  BinaryStreamReader Reader;
  FieldListDeserializer Deserializer;
  TypeVisitorCallbackPipeline Pipeline;
  CVTypeVisitor Visitor;
};

struct VisitHelper {
  VisitHelper(TypeVisitorCallbacks &Callbacks, VisitorDataSource Source)
      : Visitor((Source == VDS_BytesPresent) ? Pipeline : Callbacks) {
    if (Source == VDS_BytesPresent) {
      Pipeline.addCallbackToPipeline(Deserializer);
      Pipeline.addCallbackToPipeline(Callbacks);
    }
  }

  TypeDeserializer Deserializer;
  TypeVisitorCallbackPipeline Pipeline;
  CVTypeVisitor Visitor;
};
}

Error toolchain::codeview::visitTypeRecord(CVType &Record, TypeIndex Index,
                                      TypeVisitorCallbacks &Callbacks,
                                      VisitorDataSource Source) {
  VisitHelper V(Callbacks, Source);
  return V.Visitor.visitTypeRecord(Record, Index);
}

Error toolchain::codeview::visitTypeRecord(CVType &Record,
                                      TypeVisitorCallbacks &Callbacks,
                                      VisitorDataSource Source) {
  VisitHelper V(Callbacks, Source);
  return V.Visitor.visitTypeRecord(Record);
}

Error toolchain::codeview::visitTypeStream(const CVTypeArray &Types,
                                      TypeVisitorCallbacks &Callbacks,
                                      VisitorDataSource Source) {
  VisitHelper V(Callbacks, Source);
  return V.Visitor.visitTypeStream(Types);
}

Error toolchain::codeview::visitTypeStream(CVTypeRange Types,
                                      TypeVisitorCallbacks &Callbacks) {
  VisitHelper V(Callbacks, VDS_BytesPresent);
  return V.Visitor.visitTypeStream(Types);
}

Error toolchain::codeview::visitTypeStream(TypeCollection &Types,
                                      TypeVisitorCallbacks &Callbacks) {
  // When the internal visitor calls Types.getType(Index) the interface is
  // required to return a CVType with the bytes filled out.  So we can assume
  // that the bytes will be present when individual records are visited.
  VisitHelper V(Callbacks, VDS_BytesPresent);
  return V.Visitor.visitTypeStream(Types);
}

Error toolchain::codeview::visitMemberRecord(CVMemberRecord Record,
                                        TypeVisitorCallbacks &Callbacks,
                                        VisitorDataSource Source) {
  FieldListVisitHelper V(Callbacks, Record.Data, Source);
  return V.Visitor.visitMemberRecord(Record);
}

Error toolchain::codeview::visitMemberRecord(TypeLeafKind Kind,
                                        ArrayRef<uint8_t> Record,
                                        TypeVisitorCallbacks &Callbacks) {
  CVMemberRecord R;
  R.Data = Record;
  R.Kind = Kind;
  return visitMemberRecord(R, Callbacks, VDS_BytesPresent);
}

Error toolchain::codeview::visitMemberRecordStream(ArrayRef<uint8_t> FieldList,
                                              TypeVisitorCallbacks &Callbacks) {
  FieldListVisitHelper V(Callbacks, FieldList, VDS_BytesPresent);
  return V.Visitor.visitFieldListMemberStream(V.Reader);
}
