//===- BuiltinCAS.cpp -------------------------------------------*- C++ -*-===//
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

#include "BuiltinCAS.h"
#include "vm/core/ADT/StringExtras.h"
#include "vm/core/CAS/BuiltinObjectHasher.h"
#include "vm/core/CAS/UnifiedOnDiskCache.h"
#include "vm/core/Support/Process.h"

using namespace vm::core;
using namespace vm::core::cas;
using namespace vm::core::cas::builtin;

static StringRef getCASIDPrefix() { return "llvmcas://"; }
void BuiltinCASContext::anchor() {}

Expected<HashType> BuiltinCASContext::parseID(StringRef Reference) {
  if (!Reference.consume_front(getCASIDPrefix()))
    return createStringError(std::make_error_code(std::errc::invalid_argument),
                             "invalid cas-id '" + Reference + "'");

  // FIXME: Allow shortened references?
  if (Reference.size() != 2 * sizeof(HashType))
    return createStringError(std::make_error_code(std::errc::invalid_argument),
                             "wrong size for cas-id hash '" + Reference + "'");

  std::string Binary;
  if (!tryGetFromHex(Reference, Binary))
    return createStringError(std::make_error_code(std::errc::invalid_argument),
                             "invalid hash in cas-id '" + Reference + "'");

  assert(Binary.size() == sizeof(HashType));
  HashType Digest;
  toolchain::copy(Binary, Digest.data());
  return Digest;
}

Expected<CASID> BuiltinCAS::parseID(StringRef Reference) {
  Expected<HashType> Digest = BuiltinCASContext::parseID(Reference);
  if (!Digest)
    return Digest.takeError();

  return CASID::create(&getContext(), toStringRef(*Digest));
}

void BuiltinCASContext::printID(ArrayRef<uint8_t> Digest, raw_ostream &OS) {
  SmallString<64> Hash;
  toHex(Digest, /*LowerCase=*/true, Hash);
  OS << getCASIDPrefix() << Hash;
}

void BuiltinCASContext::printIDImpl(raw_ostream &OS, const CASID &ID) const {
  BuiltinCASContext::printID(ID.getHash(), OS);
}

const BuiltinCASContext &BuiltinCASContext::getDefaultContext() {
  static BuiltinCASContext DefaultContext;
  return DefaultContext;
}

Expected<ObjectRef> BuiltinCAS::store(ArrayRef<ObjectRef> Refs,
                                      ArrayRef<char> Data) {
  return storeImpl(BuiltinObjectHasher<HasherT>::hashObject(*this, Refs, Data),
                   Refs, Data);
}

Error BuiltinCAS::validateObject(const CASID &ID) {
  auto Ref = getReference(ID);
  if (!Ref)
    return createUnknownObjectError(ID);

  auto Handle = load(*Ref);
  if (!Handle)
    return Handle.takeError();

  auto Proxy = ObjectProxy::load(*this, *Ref, *Handle);
  SmallVector<ObjectRef> Refs;
  if (auto E = Proxy.forEachReference([&](ObjectRef Ref) -> Error {
        Refs.push_back(Ref);
        return Error::success();
      }))
    return E;

  ArrayRef<char> Data(Proxy.getData().data(), Proxy.getData().size());
  auto Hash = BuiltinObjectHasher<HasherT>::hashObject(*this, Refs, Data);
  if (!ID.getHash().equals(Hash))
    return createCorruptObjectError(ID);

  return Error::success();
}

Expected<std::unique_ptr<ondisk::UnifiedOnDiskCache>>
cas::builtin::createBuiltinUnifiedOnDiskCache(StringRef Path) {
#if LLVM_ENABLE_ONDISK_CAS
  return ondisk::UnifiedOnDiskCache::open(Path, /*SizeLimit=*/std::nullopt,
                                          BuiltinCASContext::getHashName(),
                                          sizeof(HashType));
#else
  return createStringError(inconvertibleErrorCode(), "OnDiskCache is disabled");
#endif
}
