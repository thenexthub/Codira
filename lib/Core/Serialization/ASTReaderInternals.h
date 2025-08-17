//===- ASTReaderInternals.h - AST Reader Internals --------------*- C++ -*-===//
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
//  This file provides internal definitions used in the AST reader.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_LIB_SERIALIZATION_ASTREADERINTERNALS_H
#define LANGUAGE_CORE_LIB_SERIALIZATION_ASTREADERINTERNALS_H

#include "MultiOnDiskHashTable.h"
#include "language/Core/AST/DeclarationName.h"
#include "language/Core/Basic/LLVM.h"
#include "language/Core/Serialization/ASTBitCodes.h"
#include "toolchain/ADT/DenseSet.h"
#include "toolchain/ADT/SmallVector.h"
#include "toolchain/ADT/StringRef.h"
#include "toolchain/Support/OnDiskHashTable.h"
#include <ctime>
#include <utility>

namespace language::Core {

class ASTReader;
class FileEntry;
struct HeaderFileInfo;
class HeaderSearch;
class ObjCMethodDecl;
class Module;

namespace serialization {

class ModuleFile;

namespace reader {

class ASTDeclContextNameLookupTraitBase {
protected:
  ASTReader &Reader;
  ModuleFile &F;

public:
  // Maximum number of lookup tables we allow before condensing the tables.
  static const int MaxTables = 4;

  /// The lookup result is a list of global declaration IDs.
  using data_type = SmallVector<GlobalDeclID, 4>;

  struct data_type_builder {
    data_type &Data;
    toolchain::DenseSet<GlobalDeclID> Found;

    data_type_builder(data_type &D) : Data(D) {}

    void insert(GlobalDeclID ID) {
      // Just use a linear scan unless we have more than a few IDs.
      if (Found.empty() && !Data.empty()) {
        if (Data.size() <= 4) {
          for (auto I : Found)
            if (I == ID)
              return;
          Data.push_back(ID);
          return;
        }

        // Switch to tracking found IDs in the set.
        Found.insert_range(Data);
      }

      if (Found.insert(ID).second)
        Data.push_back(ID);
    }
  };
  using hash_value_type = unsigned;
  using offset_type = unsigned;
  using file_type = ModuleFile *;

protected:
  explicit ASTDeclContextNameLookupTraitBase(ASTReader &Reader, ModuleFile &F)
      : Reader(Reader), F(F) {}

public:
  static std::pair<unsigned, unsigned>
  ReadKeyDataLength(const unsigned char *&d);

  void ReadDataIntoImpl(const unsigned char *d, unsigned DataLen,
                        data_type_builder &Val);

  static void MergeDataInto(const data_type &From, data_type_builder &To) {
    To.Data.reserve(To.Data.size() + From.size());
    for (GlobalDeclID ID : From)
      To.insert(ID);
  }

  file_type ReadFileRef(const unsigned char *&d);

  DeclarationNameKey ReadKeyBase(const unsigned char *&d);
};

/// Class that performs name lookup into a DeclContext stored
/// in an AST file.
class ASTDeclContextNameLookupTrait : public ASTDeclContextNameLookupTraitBase {
public:
  explicit ASTDeclContextNameLookupTrait(ASTReader &Reader, ModuleFile &F)
      : ASTDeclContextNameLookupTraitBase(Reader, F) {}

  using external_key_type = DeclarationName;
  using internal_key_type = DeclarationNameKey;

  static bool EqualKey(const internal_key_type &a, const internal_key_type &b) {
    return a == b;
  }

  static hash_value_type ComputeHash(const internal_key_type &Key) {
    return Key.getHash();
  }

  static internal_key_type GetInternalKey(const external_key_type &Name) {
    return Name;
  }

  internal_key_type ReadKey(const unsigned char *d, unsigned);

  void ReadDataInto(internal_key_type, const unsigned char *d,
                    unsigned DataLen, data_type_builder &Val);
};

struct DeclContextLookupTable {
  MultiOnDiskHashTable<ASTDeclContextNameLookupTrait> Table;
};

class ModuleLocalNameLookupTrait : public ASTDeclContextNameLookupTraitBase {
public:
  explicit ModuleLocalNameLookupTrait(ASTReader &Reader, ModuleFile &F)
      : ASTDeclContextNameLookupTraitBase(Reader, F) {}

  using external_key_type = std::pair<DeclarationName, const Module *>;
  using internal_key_type = std::pair<DeclarationNameKey, unsigned>;

  static bool EqualKey(const internal_key_type &a, const internal_key_type &b) {
    return a == b;
  }

  static hash_value_type ComputeHash(const internal_key_type &Key);
  static internal_key_type GetInternalKey(const external_key_type &Key);

  internal_key_type ReadKey(const unsigned char *d, unsigned);

  void ReadDataInto(internal_key_type, const unsigned char *d, unsigned DataLen,
                    data_type_builder &Val);
};

struct ModuleLocalLookupTable {
  MultiOnDiskHashTable<ModuleLocalNameLookupTrait> Table;
};

using LazySpecializationInfo = GlobalDeclID;

/// Class that performs lookup to specialized decls.
class LazySpecializationInfoLookupTrait {
  ASTReader &Reader;
  ModuleFile &F;

public:
  // Maximum number of lookup tables we allow before condensing the tables.
  static const int MaxTables = 4;

  /// The lookup result is a list of global declaration IDs.
  using data_type = SmallVector<LazySpecializationInfo, 4>;

  struct data_type_builder {
    data_type &Data;
    toolchain::DenseSet<LazySpecializationInfo> Found;

    data_type_builder(data_type &D) : Data(D) {}

    void insert(LazySpecializationInfo Info) {
      // Just use a linear scan unless we have more than a few IDs.
      if (Found.empty() && !Data.empty()) {
        if (Data.size() <= 4) {
          for (auto I : Found)
            if (I == Info)
              return;
          Data.push_back(Info);
          return;
        }

        // Switch to tracking found IDs in the set.
        Found.insert_range(Data);
      }

      if (Found.insert(Info).second)
        Data.push_back(Info);
    }
  };
  using hash_value_type = unsigned;
  using offset_type = unsigned;
  using file_type = ModuleFile *;

  using external_key_type = unsigned;
  using internal_key_type = unsigned;

  explicit LazySpecializationInfoLookupTrait(ASTReader &Reader, ModuleFile &F)
      : Reader(Reader), F(F) {}

  static bool EqualKey(const internal_key_type &a, const internal_key_type &b) {
    return a == b;
  }

  static hash_value_type ComputeHash(const internal_key_type &Key) {
    return Key;
  }

  static internal_key_type GetInternalKey(const external_key_type &Name) {
    return Name;
  }

  static std::pair<unsigned, unsigned>
  ReadKeyDataLength(const unsigned char *&d);

  internal_key_type ReadKey(const unsigned char *d, unsigned);

  void ReadDataInto(internal_key_type, const unsigned char *d, unsigned DataLen,
                    data_type_builder &Val);

  static void MergeDataInto(const data_type &From, data_type_builder &To) {
    To.Data.reserve(To.Data.size() + From.size());
    for (LazySpecializationInfo Info : From)
      To.insert(Info);
  }

  file_type ReadFileRef(const unsigned char *&d);
};

struct LazySpecializationInfoLookupTable {
  MultiOnDiskHashTable<LazySpecializationInfoLookupTrait> Table;
};

/// Base class for the trait describing the on-disk hash table for the
/// identifiers in an AST file.
///
/// This class is not useful by itself; rather, it provides common
/// functionality for accessing the on-disk hash table of identifiers
/// in an AST file. Different subclasses customize that functionality
/// based on what information they are interested in. Those subclasses
/// must provide the \c data_type type and the ReadData operation, only.
class ASTIdentifierLookupTraitBase {
public:
  using external_key_type = StringRef;
  using internal_key_type = StringRef;
  using hash_value_type = unsigned;
  using offset_type = unsigned;

  static bool EqualKey(const internal_key_type& a, const internal_key_type& b) {
    return a == b;
  }

  static hash_value_type ComputeHash(const internal_key_type& a);

  static std::pair<unsigned, unsigned>
  ReadKeyDataLength(const unsigned char*& d);

  // This hopefully will just get inlined and removed by the optimizer.
  static const internal_key_type&
  GetInternalKey(const external_key_type& x) { return x; }

  // This hopefully will just get inlined and removed by the optimizer.
  static const external_key_type&
  GetExternalKey(const internal_key_type& x) { return x; }

  static internal_key_type ReadKey(const unsigned char* d, unsigned n);
};

/// Class that performs lookup for an identifier stored in an AST file.
class ASTIdentifierLookupTrait : public ASTIdentifierLookupTraitBase {
  ASTReader &Reader;
  ModuleFile &F;

  // If we know the IdentifierInfo in advance, it is here and we will
  // not build a new one. Used when deserializing information about an
  // identifier that was constructed before the AST file was read.
  IdentifierInfo *KnownII;

  bool hasMacroDefinitionInDependencies = false;

public:
  using data_type = IdentifierInfo *;

  ASTIdentifierLookupTrait(ASTReader &Reader, ModuleFile &F,
                           IdentifierInfo *II = nullptr)
      : Reader(Reader), F(F), KnownII(II) {}

  data_type ReadData(const internal_key_type& k,
                     const unsigned char* d,
                     unsigned DataLen);

  IdentifierID ReadIdentifierID(const unsigned char *d);

  ASTReader &getReader() const { return Reader; }

  bool hasMoreInformationInDependencies() const {
    return hasMacroDefinitionInDependencies;
  }
};

/// The on-disk hash table used to contain information about
/// all of the identifiers in the program.
using ASTIdentifierLookupTable =
    toolchain::OnDiskIterableChainedHashTable<ASTIdentifierLookupTrait>;

/// Class that performs lookup for a selector's entries in the global
/// method pool stored in an AST file.
class ASTSelectorLookupTrait {
  ASTReader &Reader;
  ModuleFile &F;

public:
  struct data_type {
    SelectorID ID;
    unsigned InstanceBits;
    unsigned FactoryBits;
    bool InstanceHasMoreThanOneDecl;
    bool FactoryHasMoreThanOneDecl;
    SmallVector<ObjCMethodDecl *, 2> Instance;
    SmallVector<ObjCMethodDecl *, 2> Factory;
  };

  using external_key_type = Selector;
  using internal_key_type = external_key_type;
  using hash_value_type = unsigned;
  using offset_type = unsigned;

  ASTSelectorLookupTrait(ASTReader &Reader, ModuleFile &F)
      : Reader(Reader), F(F) {}

  static bool EqualKey(const internal_key_type& a,
                       const internal_key_type& b) {
    return a == b;
  }

  static hash_value_type ComputeHash(Selector Sel);

  static const internal_key_type&
  GetInternalKey(const external_key_type& x) { return x; }

  static std::pair<unsigned, unsigned>
  ReadKeyDataLength(const unsigned char*& d);

  internal_key_type ReadKey(const unsigned char* d, unsigned);
  data_type ReadData(Selector, const unsigned char* d, unsigned DataLen);
};

/// The on-disk hash table used for the global method pool.
using ASTSelectorLookupTable =
    toolchain::OnDiskChainedHashTable<ASTSelectorLookupTrait>;

/// Trait class used to search the on-disk hash table containing all of
/// the header search information.
///
/// The on-disk hash table contains a mapping from each header path to
/// information about that header (how many times it has been included, its
/// controlling macro, etc.). Note that we actually hash based on the size
/// and mtime, and support "deep" comparisons of file names based on current
/// inode numbers, so that the search can cope with non-normalized path names
/// and symlinks.
class HeaderFileInfoTrait {
  ASTReader &Reader;
  ModuleFile &M;

public:
  using external_key_type = FileEntryRef;

  struct internal_key_type {
    off_t Size;
    time_t ModTime;
    StringRef Filename;
    bool Imported;
  };

  using internal_key_ref = const internal_key_type &;

  using data_type = HeaderFileInfo;
  using hash_value_type = unsigned;
  using offset_type = unsigned;

  HeaderFileInfoTrait(ASTReader &Reader, ModuleFile &M)
      : Reader(Reader), M(M) {}

  static hash_value_type ComputeHash(internal_key_ref ikey);
  internal_key_type GetInternalKey(external_key_type ekey);
  bool EqualKey(internal_key_ref a, internal_key_ref b);

  static std::pair<unsigned, unsigned>
  ReadKeyDataLength(const unsigned char*& d);

  static internal_key_type ReadKey(const unsigned char *d, unsigned);

  data_type ReadData(internal_key_ref,const unsigned char *d, unsigned DataLen);

private:
  OptionalFileEntryRef getFile(const internal_key_type &Key);
};

/// The on-disk hash table used for known header files.
using HeaderFileInfoLookupTable =
    toolchain::OnDiskChainedHashTable<HeaderFileInfoTrait>;

} // namespace reader

} // namespace serialization

} // namespace language::Core

#endif // LANGUAGE_CORE_LIB_SERIALIZATION_ASTREADERINTERNALS_H
