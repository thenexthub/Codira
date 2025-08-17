//ProgramStateTrait.h - Partial implementations of ProgramStateTrait -*- C++ -*-
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
//
//  This file defines partial implementations of template specializations of
//  the class ProgramStateTrait<>.  ProgramStateTrait<> is used by ProgramState
//  to implement set/get methods for manipulating a ProgramState's
//  generic data map.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_STATICANALYZER_CORE_PATHSENSITIVE_PROGRAMSTATETRAIT_H
#define LANGUAGE_CORE_STATICANALYZER_CORE_PATHSENSITIVE_PROGRAMSTATETRAIT_H

#include "toolchain/ADT/ImmutableList.h"
#include "toolchain/ADT/ImmutableMap.h"
#include "toolchain/ADT/ImmutableSet.h"
#include "toolchain/Support/Allocator.h"
#include <cstdint>
#include <type_traits>

namespace language::Core {
namespace ento {

template <typename T, typename Enable = void> struct ProgramStatePartialTrait;

/// Declares a program state trait for type \p Type called \p Name, and
/// introduce a type named \c NameTy.
/// The macro should not be used inside namespaces.
#define REGISTER_TRAIT_WITH_PROGRAMSTATE(Name, Type)                           \
  namespace {                                                                  \
  class Name {};                                                               \
  using Name##Ty = Type;                                                       \
  }                                                                            \
  namespace language::Core {                                                            \
  namespace ento {                                                             \
  template <>                                                                  \
  struct ProgramStateTrait<Name> : public ProgramStatePartialTrait<Name##Ty> { \
    static void *GDMIndex() {                                                  \
      static int Index;                                                        \
      return &Index;                                                           \
    }                                                                          \
  };                                                                           \
  }                                                                            \
  }

  /// Declares a factory for objects of type \p Type in the program state
  /// manager. The type must provide a ::Factory sub-class. Commonly used for
  /// ImmutableMap, ImmutableSet, ImmutableList. The macro should not be used
  /// inside namespaces.
  #define REGISTER_FACTORY_WITH_PROGRAMSTATE(Type) \
    namespace language::Core { \
    namespace ento { \
      template <> \
      struct ProgramStateTrait<Type> \
        : public ProgramStatePartialTrait<Type> { \
        static void *GDMIndex() { static int Index; return &Index; } \
      }; \
    } \
    }

  /// Helper for registering a map trait.
  ///
  /// If the map type were written directly in the invocation of
  /// REGISTER_TRAIT_WITH_PROGRAMSTATE, the comma in the template arguments
  /// would be treated as a macro argument separator, which is wrong.
  /// This allows the user to specify a map type in a way that the preprocessor
  /// can deal with.
  #define CLANG_ENTO_PROGRAMSTATE_MAP(Key, Value) toolchain::ImmutableMap<Key, Value>

  /// Declares an immutable map of type \p NameTy, suitable for placement into
  /// the ProgramState. This is implementing using toolchain::ImmutableMap.
  ///
  /// \code
  /// State = State->set<Name>(K, V);
  /// const Value *V = State->get<Name>(K); // Returns NULL if not in the map.
  /// State = State->remove<Name>(K);
  /// NameTy Map = State->get<Name>();
  /// \endcode
  ///
  /// The macro should not be used inside namespaces, or for traits that must
  /// be accessible from more than one translation unit.
  #define REGISTER_MAP_WITH_PROGRAMSTATE(Name, Key, Value) \
    REGISTER_TRAIT_WITH_PROGRAMSTATE(Name, \
                                     CLANG_ENTO_PROGRAMSTATE_MAP(Key, Value))

  /// Declares an immutable map type \p Name and registers the factory
  /// for such maps in the program state, but does not add the map itself
  /// to the program state. Useful for managing lifetime of maps that are used
  /// as elements of other program state data structures.
  #define REGISTER_MAP_FACTORY_WITH_PROGRAMSTATE(Name, Key, Value) \
    using Name = toolchain::ImmutableMap<Key, Value>; \
    REGISTER_FACTORY_WITH_PROGRAMSTATE(Name)


  /// Declares an immutable set of type \p NameTy, suitable for placement into
  /// the ProgramState. This is implementing using toolchain::ImmutableSet.
  ///
  /// \code
  /// State = State->add<Name>(E);
  /// State = State->remove<Name>(E);
  /// bool Present = State->contains<Name>(E);
  /// NameTy Set = State->get<Name>();
  /// \endcode
  ///
  /// The macro should not be used inside namespaces, or for traits that must
  /// be accessible from more than one translation unit.
  #define REGISTER_SET_WITH_PROGRAMSTATE(Name, Elem) \
    REGISTER_TRAIT_WITH_PROGRAMSTATE(Name, toolchain::ImmutableSet<Elem>)

  /// Declares an immutable set type \p Name and registers the factory
  /// for such sets in the program state, but does not add the set itself
  /// to the program state. Useful for managing lifetime of sets that are used
  /// as elements of other program state data structures.
  #define REGISTER_SET_FACTORY_WITH_PROGRAMSTATE(Name, Elem) \
    using Name = toolchain::ImmutableSet<Elem>; \
    REGISTER_FACTORY_WITH_PROGRAMSTATE(Name)


  /// Declares an immutable list type \p NameTy, suitable for placement into
  /// the ProgramState. This is implementing using toolchain::ImmutableList.
  ///
  /// \code
  /// State = State->add<Name>(E); // Adds to the /end/ of the list.
  /// bool Present = State->contains<Name>(E);
  /// NameTy List = State->get<Name>();
  /// \endcode
  ///
  /// The macro should not be used inside namespaces, or for traits that must
  /// be accessible from more than one translation unit.
  #define REGISTER_LIST_WITH_PROGRAMSTATE(Name, Elem) \
    REGISTER_TRAIT_WITH_PROGRAMSTATE(Name, toolchain::ImmutableList<Elem>)

  /// Declares an immutable list of type \p Name and registers the factory
  /// for such lists in the program state, but does not add the list itself
  /// to the program state. Useful for managing lifetime of lists that are used
  /// as elements of other program state data structures.
  #define REGISTER_LIST_FACTORY_WITH_PROGRAMSTATE(Name, Elem) \
    using Name = toolchain::ImmutableList<Elem>; \
    REGISTER_FACTORY_WITH_PROGRAMSTATE(Name)


  // Partial-specialization for ImmutableMap.
  template <typename Key, typename Data, typename Info>
  struct ProgramStatePartialTrait<toolchain::ImmutableMap<Key, Data, Info>> {
    using data_type = toolchain::ImmutableMap<Key, Data, Info>;
    using context_type = typename data_type::Factory &;
    using key_type = Key;
    using value_type = Data;
    using lookup_type = const value_type *;

    static data_type MakeData(void *const *p) {
      return p ? data_type((typename data_type::TreeTy *) *p)
               : data_type(nullptr);
    }

    static void *MakeVoidPtr(data_type B) {
      return B.getRoot();
    }

    static lookup_type Lookup(data_type B, key_type K) {
      return B.lookup(K);
    }

    static data_type Set(data_type B, key_type K, value_type E,
                         context_type F) {
      return F.add(B, K, E);
    }

    static data_type Remove(data_type B, key_type K, context_type F) {
      return F.remove(B, K);
    }

    static bool Contains(data_type B, key_type K) {
      return B.contains(K);
    }

    static context_type MakeContext(void *p) {
      return *((typename data_type::Factory *) p);
    }

    static void *CreateContext(toolchain::BumpPtrAllocator& Alloc) {
      return new typename data_type::Factory(Alloc);
    }

    static void DeleteContext(void *Ctx) {
      delete (typename data_type::Factory *) Ctx;
    }
  };

  // Partial-specialization for ImmutableSet.
  template <typename Key, typename Info>
  struct ProgramStatePartialTrait<toolchain::ImmutableSet<Key, Info>> {
    using data_type = toolchain::ImmutableSet<Key, Info>;
    using context_type = typename data_type::Factory &;
    using key_type = Key;

    static data_type MakeData(void *const *p) {
      return p ? data_type((typename data_type::TreeTy *) *p)
               : data_type(nullptr);
    }

    static void *MakeVoidPtr(data_type B) {
      return B.getRoot();
    }

    static data_type Add(data_type B, key_type K, context_type F) {
      return F.add(B, K);
    }

    static data_type Remove(data_type B, key_type K, context_type F) {
      return F.remove(B, K);
    }

    static bool Contains(data_type B, key_type K) {
      return B.contains(K);
    }

    static context_type MakeContext(void *p) {
      return *((typename data_type::Factory *) p);
    }

    static void *CreateContext(toolchain::BumpPtrAllocator &Alloc) {
      return new typename data_type::Factory(Alloc);
    }

    static void DeleteContext(void *Ctx) {
      delete (typename data_type::Factory *) Ctx;
    }
  };

  // Partial-specialization for ImmutableList.
  template <typename T>
  struct ProgramStatePartialTrait<toolchain::ImmutableList<T>> {
    using data_type = toolchain::ImmutableList<T>;
    using key_type = T;
    using context_type = typename data_type::Factory &;

    static data_type Add(data_type L, key_type K, context_type F) {
      return F.add(K, L);
    }

    static bool Contains(data_type L, key_type K) {
      return L.contains(K);
    }

    static data_type MakeData(void *const *p) {
      return p ? data_type((const toolchain::ImmutableListImpl<T> *) *p)
               : data_type(nullptr);
    }

    static void *MakeVoidPtr(data_type D) {
      return const_cast<toolchain::ImmutableListImpl<T> *>(D.getInternalPointer());
    }

    static context_type MakeContext(void *p) {
      return *((typename data_type::Factory *) p);
    }

    static void *CreateContext(toolchain::BumpPtrAllocator &Alloc) {
      return new typename data_type::Factory(Alloc);
    }

    static void DeleteContext(void *Ctx) {
      delete (typename data_type::Factory *) Ctx;
    }
  };

  template <typename T> struct DefaultProgramStatePartialTraitImpl {
    using data_type = T;
    static T MakeData(void *const *P) { return P ? (T)(uintptr_t)*P : T{}; }
    static void *MakeVoidPtr(T D) { return (void *)(uintptr_t)D; }
  };

  // Partial specialization for integral types.
  template <typename T>
  struct ProgramStatePartialTrait<T,
                                  std::enable_if_t<std::is_integral<T>::value>>
      : DefaultProgramStatePartialTraitImpl<T> {};

  // Partial specialization for enums.
  template <typename T>
  struct ProgramStatePartialTrait<T, std::enable_if_t<std::is_enum<T>::value>>
      : DefaultProgramStatePartialTraitImpl<T> {};

  // Partial specialization for pointers.
  template <typename T>
  struct ProgramStatePartialTrait<T *, void>
      : DefaultProgramStatePartialTraitImpl<T *> {};

} // namespace ento
} // namespace language::Core

#endif // LANGUAGE_CORE_STATICANALYZER_CORE_PATHSENSITIVE_PROGRAMSTATETRAIT_H
