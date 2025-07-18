//===--- AnyRequest.h - Requests Instances ----------------------*- C++ -*-===//
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
//  This file defines type-erasing wrappers for requests used by the Evaluator
//  class.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_AST_ANYREQUEST_H
#define LANGUAGE_AST_ANYREQUEST_H

#include "language/Basic/SourceLoc.h"
#include "language/Basic/TypeID.h"
#include "toolchain/ADT/DenseMapInfo.h"
#include "toolchain/ADT/Hashing.h"
#include "toolchain/ADT/PointerIntPair.h"
#include <string>

namespace toolchain {
class raw_ostream;
}

namespace language {

using toolchain::hash_code;
using toolchain::hash_value;

class DiagnosticEngine;

/// A collection of functions that describe how to perform operations for a
/// specific concrete request, obtained using
/// \c AnyRequestVTable::get<ConcreteRequest>().
struct AnyRequestVTable {
  template<typename Request>
  struct Impl {
    static hash_code getHash(const void *ptr) {
      return hash_value(*static_cast<const Request *>(ptr));
    }
    static bool isEqual(const void *lhs, const void *rhs) {
      return *static_cast<const Request *>(lhs) ==
             *static_cast<const Request *>(rhs);
    }
    static void simpleDisplay(const void *ptr, toolchain::raw_ostream &out) {
      simple_display(out, *static_cast<const Request *>(ptr));
    }
    static void diagnoseCycle(const void *ptr, DiagnosticEngine &diags) {
      static_cast<const Request *>(ptr)->diagnoseCycle(diags);
    }
    static void noteCycleStep(const void *ptr, DiagnosticEngine &diags) {
      static_cast<const Request *>(ptr)->noteCycleStep(diags);
    }
    static SourceLoc getNearestLoc(const void *ptr) {
      return static_cast<const Request *>(ptr)->getNearestLoc();
    }
  };

  const uint64_t typeID;
  const std::function<hash_code(const void *)> getHash;
  const std::function<bool(const void *, const void *)> isEqual;
  const std::function<void(const void *, toolchain::raw_ostream &)> simpleDisplay;
  const std::function<void(const void *, DiagnosticEngine &)> diagnoseCycle;
  const std::function<void(const void *, DiagnosticEngine &)> noteCycleStep;
  const std::function<SourceLoc(const void *)> getNearestLoc;

  template <typename Request>
  static const AnyRequestVTable *get() {
    static const AnyRequestVTable vtable = {
        TypeID<Request>::value,
        &Impl<Request>::getHash,
        &Impl<Request>::isEqual,
        &Impl<Request>::simpleDisplay,
        &Impl<Request>::diagnoseCycle,
        &Impl<Request>::noteCycleStep,
        &Impl<Request>::getNearestLoc
    };
    return &vtable;
  }
};

/// Base class for request type-erasing wrappers.
template <typename Derived>
class AnyRequestBase {
  friend toolchain::DenseMapInfo<Derived>;

protected:
  static hash_code hashForHolder(uint64_t typeID, hash_code requestHash) {
    return hash_combine(typeID, requestHash);
  }

  enum class StorageKind : uint8_t {
    Normal,
    Empty,
    Tombstone,
  };

  /// The vtable and storage kind.
  toolchain::PointerIntPair<const AnyRequestVTable *, 2, StorageKind> vtableAndKind;

  StorageKind getStorageKind() const { return vtableAndKind.getInt(); }

  /// Whether this object is storing a value, and is not empty or a tombstone.
  bool hasStorage() const {
    switch (getStorageKind()) {
    case StorageKind::Empty:
    case StorageKind::Tombstone:
      return false;
    case StorageKind::Normal:
      return true;
    }
    toolchain_unreachable("Unhandled case in switch");
  }

  /// Retrieve the vtable to perform operations on the type-erased request.
  const AnyRequestVTable *getVTable() const {
    assert(hasStorage() && "Shouldn't be querying empty or tombstone");
    return vtableAndKind.getPointer();
  }

  AnyRequestBase(const AnyRequestVTable *vtable, StorageKind storageKind) {
    vtableAndKind.setPointer(vtable);
    vtableAndKind.setInt(storageKind);
    assert((bool)vtable == hasStorage() && "Must have a vtable with storage");
  }

  AnyRequestBase(const AnyRequestBase &other) {
    vtableAndKind = other.vtableAndKind;
  }
  AnyRequestBase &operator=(const AnyRequestBase &other) {
    vtableAndKind = other.vtableAndKind;
    return *this;
  }

private:
  Derived &asDerived() {
    return *static_cast<Derived *>(this);
  }
  const Derived &asDerived() const {
    return *static_cast<const Derived *>(this);
  }

  const void *getRawStorage() const { return asDerived().getRawStorage(); }

public:
  /// Cast to a specific (known) type.
  template<typename Request>
  const Request &castTo() const {
    assert(getVTable()->typeID == TypeID<Request>::value &&
           "Wrong type in cast");
    return *static_cast<const Request *>(getRawStorage());
  }

  /// Try casting to a specific (known) type, returning \c nullptr on
  /// failure.
  template<typename Request>
  const Request *getAs() const {
    if (getVTable()->typeID != TypeID<Request>::value)
      return nullptr;

    return static_cast<const Request *>(getRawStorage());
  }

  /// Diagnose a cycle detected for this request.
  void diagnoseCycle(DiagnosticEngine &diags) const {
    getVTable()->diagnoseCycle(getRawStorage(), diags);
  }

  /// Note that this request is part of a cycle.
  void noteCycleStep(DiagnosticEngine &diags) const {
    getVTable()->noteCycleStep(getRawStorage(), diags);
  }

  /// Get the best source location describing the parameters to this request.
  SourceLoc getNearestLoc() const {
    return getVTable()->getNearestLoc(getRawStorage());
  }

  /// Compare two instances for equality.
  friend bool operator==(const AnyRequestBase<Derived> &lhs,
                         const AnyRequestBase<Derived> &rhs) {
    // If the storage kinds don't match, we're done.
    if (lhs.getStorageKind() != rhs.getStorageKind())
      return false;

    // If the storage kinds do match, but there's no storage, they're trivially
    // equal.
    if (!lhs.hasStorage())
      return true;

    // Must be storing the same kind of request.
    if (lhs.getVTable()->typeID != rhs.getVTable()->typeID)
      return false;

    return lhs.getVTable()->isEqual(lhs.getRawStorage(), rhs.getRawStorage());
  }

  friend bool operator!=(const Derived &lhs, const Derived &rhs) {
    return !(lhs == rhs);
  }

  friend hash_code hash_value(const AnyRequestBase<Derived> &req) {
    // If there's no storage, return a trivial hash value.
    if (!req.hasStorage())
      return 1;

    auto reqHash = req.getVTable()->getHash(req.getRawStorage());
    return hashForHolder(req.getVTable()->typeID, reqHash);
  }

  friend void simple_display(toolchain::raw_ostream &out,
                             const AnyRequestBase<Derived> &req) {
    req.getVTable()->simpleDisplay(req.getRawStorage(), out);
  }
};

/// Provides a view onto a request that is stored on the stack. Objects of this
/// class must not outlive the request they reference.
///
/// Requests must be value types and provide the following API:
///
///   - Equality operator (==)
///   - Hashing support (hash_value)
///   - TypeID support (see language/Basic/TypeID.h)
///   - Display support (free function):
///       void simple_display(toolchain::raw_ostream &, const T &);
///   - Cycle diagnostics operations:
///       void diagnoseCycle(DiagnosticEngine &diags) const;
///       void noteCycleStep(DiagnosticEngine &diags) const;
///   - Source location information:
///       SourceLoc getNearestLoc() const;
///
class ActiveRequest final : public AnyRequestBase<ActiveRequest> {
  template <typename T>
  friend class AnyRequestBase;

  friend toolchain::DenseMapInfo<ActiveRequest>;

  /// Pointer to the request stored on the stack.
  const void *storage;

  /// Creates an \c ActiveRequest without storage.
  explicit ActiveRequest(StorageKind storageKind)
      : AnyRequestBase(/*vtable*/ nullptr, storageKind) {}

  const void *getRawStorage() const { return storage; }

public:
  /// Creates a new \c ActiveRequest referencing a concrete request on the
  /// stack.
  template <typename Request>
  explicit ActiveRequest(const Request &request)
      : AnyRequestBase(AnyRequestVTable::get<Request>(), StorageKind::Normal) {
    storage = &request;
  }
};

} // end namespace language

namespace toolchain {
  template<>
  struct DenseMapInfo<language::ActiveRequest> {
    using ActiveRequest = language::ActiveRequest;
    static inline ActiveRequest getEmptyKey() {
      return ActiveRequest(ActiveRequest::StorageKind::Empty);
    }
    static inline ActiveRequest getTombstoneKey() {
      return ActiveRequest(ActiveRequest::StorageKind::Tombstone);
    }
    static unsigned getHashValue(const ActiveRequest &request) {
      return hash_value(request);
    }
    static bool isEqual(const ActiveRequest &lhs, const ActiveRequest &rhs) {
      return lhs == rhs;
    }
  };

} // end namespace toolchain

#endif // LANGUAGE_AST_ANYREQUEST_H
