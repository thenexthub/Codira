//===--- Toolchain.h - Import various common LLVM datatypes ----------*- C++ -*-===//
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
/// Forward declares and imports various common LLVM datatypes.
///
//===----------------------------------------------------------------------===//

#ifndef TOOLCHAIN_SOURCEKIT_CORE_TOOLCHAIN_H
#define TOOLCHAIN_SOURCEKIT_CORE_TOOLCHAIN_H

// Do not proliferate #includes here, require clients to #include their
// dependencies.
// Casting.h has complex templates that cannot be easily forward declared.
#include "toolchain/Support/Casting.h"
#include <memory>

namespace toolchain {
  // ADT's.
  class StringRef;
  class Twine;
  template<typename T> class ArrayRef;
  template<unsigned InternalLen> class SmallString;
  template<typename T, unsigned N> class SmallVector;
  template <typename T>
  class SmallVectorImpl;

  template<typename T>
  struct SaveAndRestore;

  // Reference counting.
  template <typename T> class IntrusiveRefCntPtr;
  template <typename T> struct IntrusiveRefCntPtrInfo;
  template <class Derived> class ThreadSafeRefCountedBase;

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

namespace language {
  class ThreadSafeRefCountedBaseVPTR;
}

namespace SourceKit {
  // Casting operators.
  using toolchain::isa;
  using toolchain::cast;
  using toolchain::dyn_cast;
  using toolchain::dyn_cast_or_null;
  using toolchain::cast_or_null;
  
  // ADT's.
  using toolchain::ArrayRef;
  using toolchain::SaveAndRestore;
  using toolchain::SmallString;
  using toolchain::SmallVector;
  using toolchain::SmallVectorImpl;
  using toolchain::StringRef;
  using toolchain::Twine;

  // Reference counting.
  using toolchain::IntrusiveRefCntPtr;
  using toolchain::IntrusiveRefCntPtrInfo;
  using toolchain::ThreadSafeRefCountedBase;
  using language::ThreadSafeRefCountedBaseVPTR;
  template <typename T> class ThreadSafeRefCntPtr;

  using toolchain::raw_ostream;

  template <typename T>
  using RefPtr = IntrusiveRefCntPtr<T>;
  
} // end namespace SourceKit

#endif
