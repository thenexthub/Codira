//===- ExtractAPI/API.cpp ---------------------------------------*- C++ -*-===//
//
// Copyright (c) 2025, NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
// 
// Author: Tunjay Akbarli
// 
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
// 
//     http://www.apache.org/licenses/LICENSE-2.0
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
///
/// \file
/// This file implements the APIRecord and derived record structs,
/// and the APISet class.
///
//===----------------------------------------------------------------------===//

#include "language/Core/ExtractAPI/API.h"
#include "toolchain/ADT/StringRef.h"
#include "toolchain/Support/ErrorHandling.h"
#include <memory>

using namespace language::Core::extractapi;
using namespace toolchain;

SymbolReference::SymbolReference(const APIRecord *R)
    : Name(R->Name), USR(R->USR), Record(R) {}

APIRecord *APIRecord::castFromRecordContext(const RecordContext *Ctx) {
  switch (Ctx->getKind()) {
#define RECORD_CONTEXT(CLASS, KIND)                                            \
  case KIND:                                                                   \
    return static_cast<CLASS *>(const_cast<RecordContext *>(Ctx));
#include "language/Core/ExtractAPI/APIRecords.inc"
  default:
    return nullptr;
    // toolchain_unreachable("RecordContext derived class isn't propertly
    // implemented");
  }
}

RecordContext *APIRecord::castToRecordContext(const APIRecord *Record) {
  if (!Record)
    return nullptr;
  switch (Record->getKind()) {
#define RECORD_CONTEXT(CLASS, KIND)                                            \
  case KIND:                                                                   \
    return static_cast<CLASS *>(const_cast<APIRecord *>(Record));
#include "language/Core/ExtractAPI/APIRecords.inc"
  default:
    return nullptr;
    // toolchain_unreachable("RecordContext derived class isn't propertly
    // implemented");
  }
}

bool RecordContext::IsWellFormed() const {
  // Check that First and Last are both null or both non-null.
  return (First == nullptr) == (Last == nullptr);
}

void RecordContext::stealRecordChain(RecordContext &Other) {
  assert(IsWellFormed());
  // Other's record chain is empty, nothing to do
  if (Other.First == nullptr && Other.Last == nullptr)
    return;

  // If we don't have an empty chain append Other's chain into ours.
  if (First)
    Last->NextInContext = Other.First;
  else
    First = Other.First;

  Last = Other.Last;

  for (auto *StolenRecord = Other.First; StolenRecord != nullptr;
       StolenRecord = StolenRecord->getNextInContext())
    StolenRecord->Parent = SymbolReference(cast<APIRecord>(this));

  // Delete Other's chain to ensure we don't accidentally traverse it.
  Other.First = nullptr;
  Other.Last = nullptr;
}

void RecordContext::addToRecordChain(APIRecord *Record) const {
  assert(IsWellFormed());
  if (!First) {
    First = Record;
    Last = Record;
    return;
  }

  Last->NextInContext = Record;
  Last = Record;
}

void RecordContext::removeFromRecordChain(APIRecord *Record) {
  APIRecord *Prev = nullptr;
  for (APIRecord *Curr = First; Curr != Record; Curr = Curr->NextInContext)
    Prev = Curr;

  if (Prev)
    Prev->NextInContext = Record->NextInContext;
  else
    First = Record->NextInContext;

  if (Last == Record)
    Last = Prev;

  Record->NextInContext = nullptr;
}

APIRecord *APISet::findRecordForUSR(StringRef USR) const {
  if (USR.empty())
    return nullptr;

  auto FindIt = USRBasedLookupTable.find(USR);
  if (FindIt != USRBasedLookupTable.end())
    return FindIt->getSecond().get();

  return nullptr;
}

StringRef APISet::copyString(StringRef String) {
  if (String.empty())
    return {};

  // No need to allocate memory and copy if the string has already been stored.
  if (Allocator.identifyObject(String.data()))
    return String;

  void *Ptr = Allocator.Allocate(String.size(), 1);
  memcpy(Ptr, String.data(), String.size());
  return StringRef(reinterpret_cast<const char *>(Ptr), String.size());
}

SymbolReference APISet::createSymbolReference(StringRef Name, StringRef USR,
                                              StringRef Source) {
  return SymbolReference(copyString(Name), copyString(USR), copyString(Source));
}

void APISet::removeRecord(StringRef USR) {
  auto Result = USRBasedLookupTable.find(USR);
  if (Result != USRBasedLookupTable.end()) {
    auto *Record = Result->getSecond().get();
    auto &ParentReference = Record->Parent;
    auto *ParentRecord = const_cast<APIRecord *>(ParentReference.Record);
    if (!ParentRecord)
      ParentRecord = findRecordForUSR(ParentReference.USR);

    if (auto *ParentCtx = toolchain::cast_if_present<RecordContext>(ParentRecord)) {
      ParentCtx->removeFromRecordChain(Record);
      if (auto *RecordAsCtx = toolchain::dyn_cast<RecordContext>(Record))
        ParentCtx->stealRecordChain(*RecordAsCtx);
    } else {
      auto *It = toolchain::find(TopLevelRecords, Record);
      if (It != TopLevelRecords.end())
        TopLevelRecords.erase(It);
      if (auto *RecordAsCtx = toolchain::dyn_cast<RecordContext>(Record)) {
        for (const auto *Child = RecordAsCtx->First; Child != nullptr;
             Child = Child->getNextInContext())
          TopLevelRecords.push_back(Child);
      }
    }
    USRBasedLookupTable.erase(Result);
  }
}

void APISet::removeRecord(APIRecord *Record) { removeRecord(Record->USR); }

APIRecord::~APIRecord() {}
TagRecord::~TagRecord() {}
RecordRecord::~RecordRecord() {}
RecordFieldRecord::~RecordFieldRecord() {}
ObjCContainerRecord::~ObjCContainerRecord() {}
ObjCMethodRecord::~ObjCMethodRecord() {}
ObjCPropertyRecord::~ObjCPropertyRecord() {}
CXXMethodRecord::~CXXMethodRecord() {}

void GlobalFunctionRecord::anchor() {}
void GlobalVariableRecord::anchor() {}
void EnumConstantRecord::anchor() {}
void EnumRecord::anchor() {}
void StructFieldRecord::anchor() {}
void StructRecord::anchor() {}
void UnionFieldRecord::anchor() {}
void UnionRecord::anchor() {}
void CXXFieldRecord::anchor() {}
void CXXClassRecord::anchor() {}
void CXXConstructorRecord::anchor() {}
void CXXDestructorRecord::anchor() {}
void CXXInstanceMethodRecord::anchor() {}
void CXXStaticMethodRecord::anchor() {}
void ObjCInstancePropertyRecord::anchor() {}
void ObjCClassPropertyRecord::anchor() {}
void ObjCInstanceVariableRecord::anchor() {}
void ObjCInstanceMethodRecord::anchor() {}
void ObjCClassMethodRecord::anchor() {}
void ObjCCategoryRecord::anchor() {}
void ObjCInterfaceRecord::anchor() {}
void ObjCProtocolRecord::anchor() {}
void MacroDefinitionRecord::anchor() {}
void TypedefRecord::anchor() {}
