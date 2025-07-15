//===---- AccessibleFunction.cpp - Codira protocol conformance checking ----===//
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
// Checking and caching of Codira accessible functions.
//
//===----------------------------------------------------------------------===//

#include "ImageInspection.h"
#include "Private.h"
#include "language/Basic/Lazy.h"
#include "language/Demangling/Demangler.h"
#include "language/Runtime/AccessibleFunction.h"
#include "language/Runtime/Concurrent.h"
#include "language/Runtime/EnvironmentVariables.h"
#include "language/Runtime/Metadata.h"
#include "language/Threading/Once.h"
#include "Tracing.h"

#include <cstdint>
#include <new>

using namespace language;

#pragma mark Accessible function cache
namespace {

struct AccessibleFunctionsSection {
  const AccessibleFunctionRecord *__ptrauth_language_accessible_function_record
      Begin;
  const AccessibleFunctionRecord *__ptrauth_language_accessible_function_record
      End;

  AccessibleFunctionsSection(const AccessibleFunctionRecord *begin,
                             const AccessibleFunctionRecord *end)
      : Begin(begin), End(end) {}

  AccessibleFunctionsSection(const void *ptr, uintptr_t size) {
    auto bytes = reinterpret_cast<const char *>(ptr);
    Begin = reinterpret_cast<const AccessibleFunctionRecord *>(ptr);
    End = reinterpret_cast<const AccessibleFunctionRecord *>(bytes + size);
  }

  const AccessibleFunctionRecord *begin() const { return Begin; }
  const AccessibleFunctionRecord *end() const { return End; }
};

struct AccessibleFunctionCacheEntry {
private:
  const char *Name;
  size_t NameLength;

  const AccessibleFunctionRecord *__ptrauth_language_accessible_function_record R;

public:
  AccessibleFunctionCacheEntry(toolchain::StringRef name,
                               const AccessibleFunctionRecord *record)
      : R(record) {
    char *Name = reinterpret_cast<char *>(malloc(name.size()));
    memcpy(Name, name.data(), name.size());

    this->Name = Name;
    this->NameLength = name.size();
  }

  const AccessibleFunctionRecord *getRecord() const { return R; }

  bool matchesKey(toolchain::StringRef name) {
    return name == toolchain::StringRef{Name, NameLength};
  }

  friend toolchain::hash_code hash_value(const AccessibleFunctionCacheEntry &value) {
    return hash_value(toolchain::StringRef{value.Name, value.NameLength});
  }

  template <class... T>
  static size_t getExtraAllocationSize(T &&...ignored) {
    return 0;
  }
};

struct AccessibleFunctionsState {
  ConcurrentReadableHashMap<AccessibleFunctionCacheEntry> Cache;
  ConcurrentReadableArray<AccessibleFunctionsSection> SectionsToScan;

  AccessibleFunctionsState() {
    initializeAccessibleFunctionsLookup();
  }
};

static Lazy<AccessibleFunctionsState> Functions;

} // end anonymous namespace

TOOLCHAIN_ATTRIBUTE_UNUSED
static void _dumpAccessibleFunctionRecords(void *context) {
  auto &S = Functions.get();

  fprintf(stderr, "==== Accessible Function Records ====\n");
  int count = 0;
  for (const auto &section : S.SectionsToScan.snapshot()) {
    for (auto &record : section) {
      auto recordName =
          language::Demangle::makeSymbolicMangledNameStringRef(record.Name.get());
      auto demangledRecordName =
          language::Demangle::demangleSymbolAsString(recordName);
      fprintf(stderr, "Record name: %s\n", recordName.data());
      fprintf(stderr, "    Demangled: %s\n", demangledRecordName.c_str());
      fprintf(stderr, "    Function Ptr: %p\n", record.Function.get());
      fprintf(stderr, "    Flags.IsDistributed: %d\n", record.Flags.isDistributed());
      ++count;
    }
  }
  fprintf(stderr, "Record count: %d\n", count);
  fprintf(stderr, "==== End of Accessible Function Records ====\n");
}

static void _registerAccessibleFunctions(AccessibleFunctionsState &C,
                                         AccessibleFunctionsSection section) {
  C.SectionsToScan.push_back(section);
}

void language::addImageAccessibleFunctionsBlockCallbackUnsafe(
  const void *baseAddress, const void *functions, uintptr_t size) {
  assert(
      size % sizeof(AccessibleFunctionRecord) == 0 &&
      "accessible function section not a multiple of AccessibleFunctionRecord");

  auto &C = Functions.unsafeGetAlreadyInitialized();
  _registerAccessibleFunctions(C, AccessibleFunctionsSection{functions, size});
}

void language::addImageAccessibleFunctionsBlockCallback(
  const void *baseAddress, const void *functions, uintptr_t size) {
  Functions.get();
  addImageAccessibleFunctionsBlockCallbackUnsafe(baseAddress, functions, size);
}

static const AccessibleFunctionRecord *
_searchForFunctionRecord(AccessibleFunctionsState &S, toolchain::StringRef name) {
  auto traceState = runtime::trace::accessible_function_scan_begin(name);

  for (const auto &section : S.SectionsToScan.snapshot()) {
    for (auto &record : section) {
      auto recordName =
          language::Demangle::makeSymbolicMangledNameStringRef(record.Name.get());
      if (recordName == name) {
        return traceState.end(&record);
      }
    }
  }
  return nullptr;
}

LANGUAGE_RUNTIME_STDLIB_SPI
const AccessibleFunctionRecord *
language::runtime::language_findAccessibleFunction(const char *targetNameStart,
                                             size_t targetNameLength) {
  auto &S = Functions.get();
  toolchain::StringRef name{targetNameStart, targetNameLength};

  if (language::runtime::environment::LANGUAGE_DUMP_ACCESSIBLE_FUNCTIONS()) {
    static language::once_t dumpAccessibleFunctionsToken;
    language::once(dumpAccessibleFunctionsToken, _dumpAccessibleFunctionRecords, nullptr);
  }

  // Look for an existing entry.
  {
    auto snapshot = S.Cache.snapshot();
    if (auto E = snapshot.find(name)) {
      return E->getRecord();
    }
  }

  // If entry doesn't exist (either record doesn't exist, hasn't been loaded, or
  // requested yet), let's try to find it and add to the cache.

  auto *record = _searchForFunctionRecord(S, name);
  if (record) {
    S.Cache.getOrInsert(
        name, [&](AccessibleFunctionCacheEntry *entry, bool created) {
          if (created)
            ::new (entry) AccessibleFunctionCacheEntry{name, record};
          return true;
        });
  }

  return record;
}
