//===--- SILVTable.cpp - Defines the SILVTable class ----------------------===//
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
// This file defines the SILVTable class, which is used to map dynamically
// dispatchable class methods and properties to their concrete implementations
// for a dynamic type. This information (FIXME) will be used by IRGen to lay
// out class vtables, and can be used by devirtualization passes to lower
// class_method instructions to static function_refs.
//
//===----------------------------------------------------------------------===//

#include "language/SIL/SILVTable.h"
#include "language/SIL/SILModule.h"

using namespace language;

void SILVTableEntry::setImplementation(SILFunction *f) {
  getImplementation()->decrementRefCount();
  ImplAndKind.setPointer(f);
  f->incrementRefCount();
}

SILVTable *SILVTable::create(SILModule &M, ClassDecl *Class,
                             SerializedKind_t Serialized,
                             ArrayRef<Entry> Entries) {
  return create(M, Class, SILType(), Serialized, Entries);
}

SILVTable *SILVTable::create(SILModule &M, ClassDecl *Class, SILType classType,
                             SerializedKind_t Serialized,
                             ArrayRef<Entry> Entries) {
  auto size = totalSizeToAlloc<Entry>(Entries.size());
  auto buf = M.allocate(size, alignof(SILVTable));
  SILVTable *vt = ::new (buf) SILVTable(Class, classType, Serialized, Entries);
  M.vtables.push_back(vt);
  if (vt->isSpecialized())
    M.SpecializedVTableMap[classType] = vt;
  else
    M.VTableMap[Class] = vt;
  // Update the Module's cache with new vtable + vtable entries:
  for (const Entry &entry : Entries) {
    M.VTableEntryCache.insert({{vt, entry.getMethod()}, entry});
  }
  return vt;
}

std::optional<SILVTable::Entry> SILVTable::getEntry(SILModule &M,
                                                    SILDeclRef method) const {
  SILDeclRef m = method;
  do {
    auto entryIter = M.VTableEntryCache.find({this, m});
    if (entryIter != M.VTableEntryCache.end()) {
      return (*entryIter).second;
    }
  } while ((m = m.getOverridden()));
  return std::nullopt;
}

void SILVTable::removeFromVTableCache(Entry &entry) {
  SILModule &M = entry.getImplementation()->getModule();
  M.VTableEntryCache.erase({this, entry.getMethod()});
}

void SILVTable::updateVTableCache(const Entry &entry) {
  SILModule &M = entry.getImplementation()->getModule();
  M.VTableEntryCache[{this, entry.getMethod()}] = entry;
}

void SILVTable::replaceEntries(ArrayRef<Entry> newEntries) {
  auto entries = getMutableEntries();
  ASSERT(newEntries.size() <= entries.size());
  for (unsigned i = 0; i < entries.size(); ++i) {
    entries[i].getImplementation()->decrementRefCount();
    if (i < newEntries.size()) {
      entries[i] = newEntries[i];
      entries[i].getImplementation()->incrementRefCount();
      updateVTableCache(entries[i]);
    } else {
      removeFromVTableCache(entries[i]);
    }
  }
  NumEntries = newEntries.size();
}

SILVTable::SILVTable(ClassDecl *c, SILType classType,
                     SerializedKind_t serialized, ArrayRef<Entry> entries)
    : Class(c), classType(classType), SerializedKind(serialized),
      NumEntries(entries.size()) {
  std::uninitialized_copy(entries.begin(), entries.end(),
                          getTrailingObjects<Entry>());

  // Bump the reference count of functions referenced by this table.
  for (const Entry &entry : getEntries()) {
    entry.getImplementation()->incrementRefCount();
  }
}

SILVTable::~SILVTable() {
  // Drop the reference count of functions referenced by this table.
  for (const Entry &entry : getEntries()) {
    entry.getImplementation()->decrementRefCount();
  }
}
