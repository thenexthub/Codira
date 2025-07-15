//===--- SILVTable.h - Defines the SILVTable class --------------*- C++ -*-===//
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
// for a dynamic type. This information is used by IRGen to emit class vtables,
// by the devirtualization pass to promote class_method instructions to static
// function_refs.
//
// Note that vtable layout itself is implemented in SILVTableLayout.h and is
// independent of the SILVTable; in general, for a class from another module we
// might not have a SILVTable to deserialize, and for a class in a different
// translation in the same module the SILVTable is not available either.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_SIL_SILVTABLE_H
#define LANGUAGE_SIL_SILVTABLE_H

#include "language/SIL/SILAllocated.h"
#include "language/SIL/SILDeclRef.h"
#include "language/SIL/SILFunction.h"
#include <algorithm>
#include <optional>

namespace language {

class ClassDecl;
enum SerializedKind_t : uint8_t;
class SILFunction;
class SILModule;

// TODO: Entry should include substitutions needed to invoke an overridden
// generic base class method.
class SILVTableEntry {
  /// The declaration reference to the least-derived method visible through
  /// the class.
  SILDeclRef Method;

  /// The function which implements the method for the class and the entry kind.
  toolchain::PointerIntPair<SILFunction *, 2, unsigned> ImplAndKind;

  bool IsNonOverridden;

public:
  enum Kind : uint8_t {
    /// The vtable entry is for a method defined directly in this class.
    Normal,
    /// The vtable entry is inherited from the superclass.
    Inherited,
    /// The vtable entry is inherited from the superclass, and overridden
    /// in this class.
    Override,

    // Please update the PointerIntPair above if you add/remove enums.
  };

  SILVTableEntry() : ImplAndKind(nullptr, Kind::Normal),
                     IsNonOverridden(false) {}

  SILVTableEntry(SILDeclRef Method, SILFunction *Implementation, Kind TheKind,
                 bool NonOverridden)
      : Method(Method), ImplAndKind(Implementation, TheKind),
        IsNonOverridden(NonOverridden) {}

  SILDeclRef getMethod() const { return Method; }

  Kind getKind() const { return Kind(ImplAndKind.getInt()); }
  void setKind(Kind kind) { ImplAndKind.setInt(kind); }

  bool isNonOverridden() const { return IsNonOverridden; }
  void setNonOverridden(bool value) { IsNonOverridden = value; }

  SILFunction *getImplementation() const { return ImplAndKind.getPointer(); }
  void setImplementation(SILFunction *f);
  
  void print(toolchain::raw_ostream &os) const;
  
  bool operator==(const SILVTableEntry &e) const {
    return Method == e.Method
      && getImplementation() == e.getImplementation()
      && getKind() == e.getKind()
      && isNonOverridden() == e.isNonOverridden();
  }
  
  bool operator!=(const SILVTableEntry &e) const {
    return !(*this == e);
  }
};

/// A mapping from each dynamically-dispatchable method of a class to the
/// SILFunction that implements the method for that class.
/// Note that dead methods are completely removed from the vtable.
class SILVTable final : public SILAllocated<SILVTable>,
                        toolchain::TrailingObjects<SILVTable, SILVTableEntry> {
  friend TrailingObjects;

public:
  using Entry = SILVTableEntry;

  // Disallow copying into temporary objects.
  SILVTable(const SILVTable &other) = delete;
  SILVTable &operator=(const SILVTable &) = delete;

private:
  /// The ClassDecl mapped to this VTable.
  ClassDecl *Class;

  /// The class type if this is a specialized vtable, otherwise null.
  SILType classType;

  /// Whether or not this vtable is serialized, which allows
  /// devirtualization from another module.
  unsigned SerializedKind : 2;

  /// The number of SILVTables entries.
  unsigned NumEntries : 31;

  /// Private constructor. Create SILVTables by calling SILVTable::create.
  SILVTable(ClassDecl *c, SILType classType, SerializedKind_t serialized,
            ArrayRef<Entry> entries);

public:
  ~SILVTable();

  /// Create a new SILVTable with the given method-to-implementation mapping.
  /// The SILDeclRef keys should reference the most-overridden members available
  /// through the class.
  static SILVTable *create(SILModule &M, ClassDecl *Class, SILType classType,
                           SerializedKind_t Serialized,
                           ArrayRef<Entry> Entries);

  /// Create a new SILVTable with the given method-to-implementation mapping.
  /// The SILDeclRef keys should reference the most-overridden members available
  /// through the class.
  static SILVTable *create(SILModule &M, ClassDecl *Class,
                           SerializedKind_t Serialized,
                           ArrayRef<Entry> Entries);

  /// Return the class that the vtable represents.
  ClassDecl *getClass() const { return Class; }

  bool isSpecialized() const {
    return !classType.isNull();
  }
  SILType getClassType() const { return classType; }

  /// Returns true if this vtable is going to be (or was) serialized.
  bool isSerialized() const {
    return SerializedKind_t(SerializedKind) == IsSerialized;
  }

  bool isAnySerialized() const {
    return SerializedKind_t(SerializedKind) == IsSerialized ||
           SerializedKind_t(SerializedKind) == IsSerializedForPackage;
  }

  SerializedKind_t getSerializedKind() const {
    return SerializedKind_t(SerializedKind);
  }
  /// Sets the serialized flag.
  void setSerializedKind(SerializedKind_t serializedKind) {
    SerializedKind = serializedKind;
  }

  /// Return all of the method entries.
  ArrayRef<Entry> getEntries() const {
    return {getTrailingObjects<SILVTableEntry>(), NumEntries};
  }

  /// Return all of the method entries mutably.
  /// If you do modify entries, make sure to invoke `updateVTableCache` to update the
  /// SILModule's cache entry.
  MutableArrayRef<Entry> getMutableEntries() {
    return {getTrailingObjects<SILVTableEntry>(), NumEntries};
  }
                          
  void updateVTableCache(const Entry &entry);

  /// Look up the implementation function for the given method.
  std::optional<Entry> getEntry(SILModule &M, SILDeclRef method) const;

  /// Removes entries from the vtable.
  /// \p predicate Returns true if the passed entry should be removed.
  template <typename Predicate> void removeEntries_if(Predicate predicate) {
    auto Entries = getMutableEntries();
    Entry *end = std::remove_if(
        Entries.begin(), Entries.end(), [&](Entry &entry) -> bool {
          if (predicate(entry)) {
            entry.getImplementation()->decrementRefCount();
            removeFromVTableCache(entry);
            return true;
          }
          return false;
        });
    NumEntries = std::distance(Entries.begin(), end);
  }

  void replaceEntries(ArrayRef<Entry> newEntries);

  /// Verify that the vtable is well-formed for the given class.
  void verify(const SILModule &M) const;

  /// Print the vtable.
  void print(toolchain::raw_ostream &OS, bool Verbose = false) const;
  void dump() const;

private:
  void removeFromVTableCache(Entry &entry);
};

} // end language namespace

//===----------------------------------------------------------------------===//
// ilist_traits for SILVTable
//===----------------------------------------------------------------------===//

namespace toolchain {

template <>
struct ilist_traits<::language::SILVTable> :
public ilist_node_traits<::language::SILVTable> {
  using SILVTable = ::language::SILVTable;

  static void deleteNode(SILVTable *VT) { VT->~SILVTable(); }

private:
  void createNode(const SILVTable &);
};

} // end toolchain namespace

#endif
