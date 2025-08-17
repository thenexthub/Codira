//===--- LLVM.h - Import various common LLVM datatypes ----------*- C++ -*-===//
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
/// \file
/// \brief Forward declares and imports various common LLVM datatypes.
///
//===----------------------------------------------------------------------===//

#ifndef INDEXSTOREDB_SUPPORT_LLVM_H
#define INDEXSTOREDB_SUPPORT_LLVM_H

// Do not proliferate #includes here, require clients to #include their
// dependencies.
// Casting.h has complex templates that cannot be easily forward declared.
#include <IndexStoreDB_LLVMSupport/toolchain_Support_Casting.h>
// None.h includes an enumerator that is desired & cannot be forward declared
// without a definition of NoneType.
#include <IndexStoreDB_LLVMSupport/toolchain_ADT_None.h>
#include <memory>

namespace toolchain {
  // ADT's.
  class StringRef;
  class Twine;
  template<typename T> class ArrayRef;
  template<typename Fn> class function_ref;
  template<unsigned InternalLen> class SmallString;
  template<typename T, unsigned N> class SmallVector;
  template<typename T> class SmallVectorImpl;
  template<typename T> class Optional;

  template<typename T>
  struct SaveAndRestore;

  // Reference counting.
  template <typename T> class IntrusiveRefCntPtr;
  template <typename T> struct IntrusiveRefCntPtrInfo;

  class raw_ostream;
  // TODO: DenseMap, ...

  template<class To, class From>
  struct cast_retty_impl<To, std::shared_ptr<From>> {
    typedef std::shared_ptr<To> ret_type;
  };

  template <typename To, typename From, typename Enabler>
  struct isa_impl<To, std::shared_ptr<From>, Enabler> {
    static inline bool doit(const std::shared_ptr<From> &Val) {
      return To::classof(Val.get());
    }
  };

  template<class To, class From>
  struct cast_convert_val<To, std::shared_ptr<From>, std::shared_ptr<From>> {
    static typename cast_retty<To, std::shared_ptr<From>>::ret_type doit(
        const std::shared_ptr<From> &Val) {
      return std::shared_ptr<To>(Val, static_cast<To*>(Val.get()));
    }
  };
}

namespace IndexStoreDB {
  // Casting operators.
  using toolchain::isa;
  using toolchain::cast;
  using toolchain::dyn_cast;
  using toolchain::dyn_cast_or_null;
  using toolchain::cast_or_null;

  // ADT's.
  using toolchain::StringRef;
  using toolchain::Twine;
  using toolchain::ArrayRef;
  using toolchain::function_ref;
  using toolchain::SmallString;
  using toolchain::SmallVector;
  using toolchain::SmallVectorImpl;
  using toolchain::SaveAndRestore;
  using toolchain::Optional;
  using toolchain::None;

  // Reference counting.
  using toolchain::IntrusiveRefCntPtr;
  using toolchain::IntrusiveRefCntPtrInfo;
  template <typename T> class ThreadSafeRefCntPtr;

  using toolchain::raw_ostream;

  template <typename T>
  using RefPtr = IntrusiveRefCntPtr<T>;

} // end namespace IndexStoreDB.

#endif
