//===--- UIDRegistry.cpp --------------------------------------------------===//
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

#include "SourceKit/Support/Concurrency.h"
#include "SourceKit/Support/UIdent.h"
#include "llvm/ADT/StringMap.h"
#include "llvm/Support/Allocator.h"
#include "llvm/Support/raw_ostream.h"
#include <mutex>
#include <vector>

using namespace SourceKit;
using llvm::StringRef;

namespace {
class UIDRegistryImpl {
  typedef llvm::StringMap<void *, llvm::BumpPtrAllocator> HashTableTy;
  typedef llvm::StringMapEntry<void *> EntryTy;
  HashTableTy HashTable;
  WorkQueue Queue{ WorkQueue::Dequeuing::Concurrent, "UIDRegistryImpl" };

public:

  void *get(StringRef Str);
  static StringRef getName(void *Ptr);
  static void setTag(void *Ptr, void *Tag);
  static void *getTag(void *Ptr);
};
} // end anonymous namespace

static UIDRegistryImpl *getGlobalRegistry() {
  static UIDRegistryImpl *GlobalRegistry = 0;
  if (!GlobalRegistry) {
    static std::once_flag flag;
    std::call_once(flag, [](){ GlobalRegistry = new UIDRegistryImpl(); });
  }
  return GlobalRegistry;
}

UIdent::UIdent(llvm::StringRef Str) {
  Ptr = getGlobalRegistry()->get(Str);
}

llvm::StringRef UIdent::getName() const {
  if (isInvalid())
    return StringRef();
  return UIDRegistryImpl::getName(Ptr);
}

const char *UIdent::c_str() const {
  if (isInvalid())
    return "";
  return getName().begin();
}

void UIdent::setTag(void *Tag) {
  assert(isValid());
  UIDRegistryImpl::setTag(Ptr, Tag);
}

void *UIdent::getTag() const {
  assert(isValid());
  return UIDRegistryImpl::getTag(Ptr);
}

void UIdent::dump() const {
  print(llvm::errs());
}

void UIdent::print(llvm::raw_ostream &OS) const {
  if (isInvalid())
    OS << "<<INVALID>>";
  else
    OS << getName();
}

void *UIDRegistryImpl::get(StringRef Str) {
  assert(!Str.empty());
  assert(!Str.contains(' '));
  EntryTy *Ptr = 0;
  Queue.dispatchSync([&]{
    HashTableTy::iterator It = HashTable.find(Str);
    if (It != HashTable.end())
      Ptr = &(*It);
  });

  if (Ptr == 0) {
    Queue.dispatchBarrierSync([&]{
      EntryTy &Entry = *HashTable.insert(std::make_pair(Str, nullptr)).first;
      Ptr = &Entry;
    });
  }

  return Ptr;
}

StringRef UIDRegistryImpl::getName(void *Ptr) {
  EntryTy *Entry = static_cast<EntryTy*>(Ptr);
  return Entry->getKey();
}

void UIDRegistryImpl::setTag(void *Ptr, void *Tag) {
  EntryTy *Entry = static_cast<EntryTy*>(Ptr);
  Entry->setValue(Tag);
}

void *UIDRegistryImpl::getTag(void *Ptr) {
  EntryTy *Entry = static_cast<EntryTy*>(Ptr);
  return Entry->getValue();
}
