//===----------------------------------------------------------------------===//
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

#include "language/IDE/CodeCompletionCache.h"
#include "language/Basic/Assertions.h"
#include "language/Basic/Cache.h"
#include "language/Basic/StringExtras.h"
#include "toolchain/ADT/APInt.h"
#include "toolchain/ADT/DenseMap.h"
#include "toolchain/ADT/Hashing.h"
#include "toolchain/ADT/StringMap.h"
#include "toolchain/Support/EndianStream.h"
#include "toolchain/Support/FileSystem.h"
#include "toolchain/Support/MemoryBuffer.h"
#include "toolchain/Support/Path.h"

using namespace language;
using namespace ide;

namespace language {
  namespace ide {
    struct CodeCompletionCacheImpl {
      using Key = CodeCompletionCache::Key;
      using Value = CodeCompletionCache::Value;
      using ValueRefCntPtr = CodeCompletionCache::ValueRefCntPtr;
      sys::Cache<Key, ValueRefCntPtr> TheCache{"language.libIDE.CodeCompletionCache"};
    };
  } // end namespace ide
} // end namespace language

namespace language {
  namespace sys {
    template<>
    struct CacheValueCostInfo<language::ide::CodeCompletionCacheImpl::Value> {
      static size_t
      getCost(const language::ide::CodeCompletionCacheImpl::Value &V) {
        return V.Allocator->getTotalMemory();
      }
    };
  } // namespace sys
} // namespace language

CodeCompletionCache::ValueRefCntPtr CodeCompletionCache::createValue() {
  return ValueRefCntPtr(new Value);
}

std::optional<CodeCompletionCache::ValueRefCntPtr>
CodeCompletionCache::get(const Key &K) {
  auto &TheCache = Impl->TheCache;
  std::optional<ValueRefCntPtr> V = TheCache.get(K);
  if (V) {
    // Check whether V is up to date.
    toolchain::sys::fs::file_status ModuleStatus;
    if (toolchain::sys::fs::status(K.ModuleFilename, ModuleStatus) ||
        V.value()->ModuleModificationTime !=
        ModuleStatus.getLastModificationTime()) {
      // Cache is stale.
      V = std::nullopt;
      TheCache.remove(K);
    }
  } else if (nextCache && (V = nextCache->get(K))) {
    // Hit the chained cache. Update our own cache to match.
    setImpl(K, *V, /*setChain*/ false);
  }
  return V;
}

void CodeCompletionCache::setImpl(const Key &K, ValueRefCntPtr V,
                                  bool setChain) {
  {
    assert(!K.ModuleFilename.empty());

    toolchain::sys::fs::file_status ModuleStatus;
    if (toolchain::sys::fs::status(K.ModuleFilename, ModuleStatus)) {
      V->ModuleModificationTime = std::chrono::system_clock::now();
      return;
    } else {
      V->ModuleModificationTime = ModuleStatus.getLastModificationTime();
    }
  }
  Impl->TheCache.set(K, V);

  // FIXME: we could write the results to disk in the background, since they're
  // immutable at this point.
  if (nextCache && setChain)
    nextCache->set(K, V);
}

CodeCompletionCache::CodeCompletionCache(OnDiskCodeCompletionCache *nextCache)
    : Impl(new CodeCompletionCacheImpl()), nextCache(nextCache) {}

CodeCompletionCache::~CodeCompletionCache() {}

/// A version number for the format of the serialized code completion results.
///
/// This should be incremented any time we commit a change to the format of the
/// cached results. This isn't expected to change very often.
static constexpr uint32_t onDiskCompletionCacheVersion =
    12; // Removed 'IsAsync'.

/// Deserializes CodeCompletionResults from \p in and stores them in \p V.
/// \see writeCacheModule.
static bool readCachedModule(toolchain::MemoryBuffer *in,
                             const CodeCompletionCache::Key &K,
                             CodeCompletionCache::Value &V,
                             bool allowOutOfDate = false) {
  const char *cursor = in->getBufferStart();
  const char *end = in->getBufferEnd();

  auto read32le = [end](const char *&cursor) {
    auto result = toolchain::support::endian::read32le(cursor);
    cursor += sizeof(result);
    assert(cursor <= end);
    (void)end;
    return result;
  };

  // HEADER
  {
    auto version = read32le(cursor);
    if (version != onDiskCompletionCacheVersion)
      return false; // File written with different format.

    auto mtime = toolchain::support::endian::read64le(cursor);
    cursor += sizeof(mtime);

    // Check the module file's last modification time.
    if (!allowOutOfDate) {
      toolchain::sys::fs::file_status status;
      if (toolchain::sys::fs::status(K.ModuleFilename, status) ||
          status.getLastModificationTime().time_since_epoch().count() !=
          std::chrono::nanoseconds(mtime).count()) {
        return false; // Out of date, or doesn't exist.
      }
    }
  }

  // DEBUG INFO
  cursor += read32le(cursor); // Skip the whole debug section.

  // Get the size of the various sections.
  auto resultSize = read32le(cursor);
  const char *resultEnd = cursor + resultSize;
  const char *chunks = resultEnd;
  auto chunkSize = read32le(chunks);
  const char *strings = chunks + chunkSize;
  auto stringsSize = read32le(strings);
  const char *types = strings + stringsSize;
  auto typesSize = read32le(types);
  assert(types + typesSize == end && "incorrect file size");
  (void)typesSize; // so it is not seen as "unused" in release builds.

  // STRINGS
  toolchain::DenseMap<uint32_t, NullTerminatedStringRef> knownStrings;
  auto getString = [&](uint32_t index) -> NullTerminatedStringRef {
    if (index == ~0u)
      return "";
    auto found = knownStrings.find(index);
    if (found != knownStrings.end()) {
      return found->second;
    }

    const char *p = strings + index;
    size_t size = read32le(p);
    auto str = NullTerminatedStringRef(StringRef(p, size), *V.Allocator);
    knownStrings[index] = str;
    return str;
  };

  // TYPES
  toolchain::DenseMap<uint32_t, const USRBasedType *> knownTypes;
  std::function<const USRBasedType *(uint32_t)> getType =
      [&](uint32_t index) -> const USRBasedType * {
    auto found = knownTypes.find(index);
    if (found != knownTypes.end()) {
      return found->second;
    }
    const char *p = types + index;
    auto usrLength = read32le(p);
    auto usr = StringRef(p, usrLength);
    p += usrLength;
    auto supertypesCount = read32le(p);
    std::vector<const USRBasedType *> supertypes;
    supertypes.reserve(supertypesCount);
    for (unsigned i = 0; i < supertypesCount; i++) {
      auto supertypeIndex = read32le(p);
      supertypes.push_back(getType(supertypeIndex));
    }
    auto customAttributeKinds = OptionSet<CustomAttributeKind, uint8_t>(*p++);
    const USRBasedType *res = USRBasedType::fromUSR(
        usr, supertypes, customAttributeKinds, V.USRTypeArena);
    knownTypes[index] = res;
    return res;
  };

  // CHUNKS
  auto getCompletionString = [&](uint32_t chunkIndex) {
    const char *p = chunks + chunkIndex;
    auto len = read32le(p);
    using Chunk = CodeCompletionString::Chunk;
    SmallVector<Chunk, 32> chunkList;
    for (unsigned j = 0; j < len; ++j) {
      auto kind = static_cast<Chunk::ChunkKind>(*p++);
      auto nest = *p++;
      auto isAnnotation = static_cast<bool>(*p++);
      auto textIndex = read32le(p);
      auto text = getString(textIndex);

      if (Chunk::chunkHasText(kind)) {
        chunkList.push_back(
            Chunk::createWithText(kind, nest, text, isAnnotation));
      } else {
        chunkList.push_back(Chunk::createSimple(kind, nest, isAnnotation));
      }
    }

    return CodeCompletionString::create(*V.Allocator, chunkList);
  };

  // RESULTS
  while (cursor != resultEnd) {
    auto kind = static_cast<CodeCompletionResultKind>(*cursor++);
    auto associatedKind = static_cast<uint8_t>(*cursor++);
    auto opKind = static_cast<CodeCompletionOperatorKind>(*cursor++);
    auto roles = CodeCompletionMacroRoles(*cursor++);
    auto notRecommended =
        static_cast<ContextFreeNotRecommendedReason>(*cursor++);
    auto diagSeverity =
        static_cast<CodeCompletionDiagnosticSeverity>(*cursor++);
    auto isSystem = static_cast<bool>(*cursor++);
    auto hasAsyncAlternative = static_cast<bool>(*cursor++);
    auto chunkIndex = read32le(cursor);
    auto moduleIndex = read32le(cursor);
    auto briefDocIndex = read32le(cursor);
    auto diagMessageIndex = read32le(cursor);
    auto filterNameIndex = read32le(cursor);
    auto nameForDiagnosticsIndex = read32le(cursor);

    auto assocUSRCount = read32le(cursor);
    SmallVector<NullTerminatedStringRef, 4> assocUSRs;
    for (unsigned i = 0; i < assocUSRCount; ++i) {
      assocUSRs.push_back(getString(read32le(cursor)));
    }

    auto resultTypesCount = read32le(cursor);
    SmallVector<const USRBasedType *, 1> resultTypes;
    resultTypes.reserve(resultTypesCount);
    for (size_t i = 0; i < resultTypesCount; i++) {
      resultTypes.push_back(getType(read32le(cursor)));
    }

    CodeCompletionString *string = getCompletionString(chunkIndex);
    auto moduleName = getString(moduleIndex);
    auto briefDocComment = getString(briefDocIndex);
    auto diagMessage = getString(diagMessageIndex);
    auto filterName = getString(filterNameIndex);
    auto nameForDiagnostics = getString(nameForDiagnosticsIndex);

    ContextFreeCodeCompletionResult *result =
        new (*V.Allocator) ContextFreeCodeCompletionResult(
            kind, associatedKind, opKind, roles, isSystem,
                                                           hasAsyncAlternative, string, moduleName, briefDocComment,
            toolchain::ArrayRef(assocUSRs).copy(*V.Allocator),
            CodeCompletionResultType(resultTypes), notRecommended, diagSeverity,
            diagMessage, filterName, nameForDiagnostics);

    V.Results.push_back(result);
  }

  return true;
}

/// Writes the code completion results from the sink for \p V to \p out.
///
/// The high-level format is:
///
///   HEADER
///     * version, which **must be bumped** if we change the format!
///     * mtime for the module file
///
///   KEY
///     * the original CodeCompletionCache::Key, used for debugging the cache.
///
///   RESULTS
///     * A length-prefixed array of fixed size CodeCompletionResults.
///     * Contains offsets into CHUNKS and STRINGS.
///
///   CHUNKS
///     * A length-prefixed array of CodeCompletionStrings.
///     * Each CodeCompletionString is a length-prefixed array of fixed size
///       CodeCompletionString::Chunks.
///
///   STRINGS
///     * A blob of length-prefixed strings referred to in CHUNKS or RESULTS.
static void writeCachedModule(toolchain::raw_ostream &out,
                              const CodeCompletionCache::Key &K,
                              CodeCompletionCache::Value &V) {
  toolchain::support::endian::Writer LE(out, toolchain::endianness::little);

  // HEADER
  // Metadata required for reading the completions.
  LE.write(onDiskCompletionCacheVersion);           // Version
  auto mtime = V.ModuleModificationTime.time_since_epoch().count();
  LE.write(mtime);                                  // Mtime for module file

  // KEY
  // We don't need the stored key to load the results, but it is useful if we
  // want to debug the cache itself.
  {
    SmallString<256> scratch;
    toolchain::raw_svector_ostream OSS(scratch);
    OSS << K.ModuleFilename << "\0";
    OSS << K.ModuleName << "\0";
    toolchain::support::endian::Writer OSSLE(OSS, toolchain::endianness::little);
    OSSLE.write(K.AccessPath.size());
    for (StringRef p : K.AccessPath)
      OSS << p << "\0";
    OSSLE.write(K.ResultsHaveLeadingDot);
    OSSLE.write(K.ForTestableLookup);
    OSSLE.write(K.ForPrivateImportLookup);
    OSSLE.write(K.AddInitsInToplevel);
    OSSLE.write(K.AddCallWithNoDefaultArgs);
    OSSLE.write(K.Annotated);
    LE.write(static_cast<uint32_t>(OSS.tell()));   // Size of debug info
    out.write(OSS.str().data(), OSS.str().size()); // Debug info blob
  }

  // String streams for writing to the CHUNKS and STRINGS sections.
  std::string results_;
  toolchain::raw_string_ostream results(results_);
  std::string chunks_;
  toolchain::raw_string_ostream chunks(chunks_);
  toolchain::support::endian::Writer chunksLE(chunks, toolchain::endianness::little);
  std::string strings_;
  toolchain::raw_string_ostream strings(strings_);
  toolchain::StringMap<uint32_t> knownStrings;
  std::string types_;
  toolchain::raw_string_ostream types(types_);
  toolchain::DenseMap<const USRBasedType *, uint32_t> knownTypes;

  auto addString = [&strings, &knownStrings](StringRef str) {
    if (str.empty())
      return ~0u;
    auto found = knownStrings.find(str);
    if (found != knownStrings.end()) {
      return found->second;
    }
    auto size = strings.tell();
    toolchain::support::endian::Writer LE(strings, toolchain::endianness::little);
    LE.write(static_cast<uint32_t>(str.size()));
    strings << str;
    knownStrings[str] = size;
    return static_cast<uint32_t>(size);
  };

  std::function<uint32_t(const USRBasedType *)> addType =
      [&types, &knownTypes, &addType](const USRBasedType *type) -> uint32_t {
    auto found = knownTypes.find(type);
    if (found != knownTypes.end()) {
      return found->second;
    }
    std::vector<uint32_t> supertypeIndicies;
    // IMPORTANT: To compute the supertype indicies, we might need to add
    // entries to the type table by calling addType recursively. Thus, we must
    // perform this calculation before writing any bytes of this type to the
    // types table.
    auto supertypes = type->getSupertypes();
    supertypeIndicies.reserve(supertypes.size());
    for (auto supertype : supertypes) {
      supertypeIndicies.push_back(addType(supertype));
    }

    auto size = types.tell();
    toolchain::support::endian::Writer LE(types, toolchain::endianness::little);
    StringRef USR = type->getUSR();
    LE.write(static_cast<uint32_t>(USR.size()));
    types << USR;
    LE.write(static_cast<uint32_t>(supertypeIndicies.size()));
    for (auto supertypeIndex : supertypeIndicies) {
      LE.write(static_cast<uint32_t>(supertypeIndex));
    }
    OptionSet<CustomAttributeKind, uint8_t> customAttributeKinds =
        type->getCustomAttributeKinds();
    LE.write(static_cast<uint8_t>(customAttributeKinds.toRaw()));
    knownTypes[type] = size;
    return static_cast<uint32_t>(size);
  };

  auto addCompletionString = [&](const CodeCompletionString *str) {
    auto size = chunks.tell();
    chunksLE.write(static_cast<uint32_t>(str->getChunks().size()));
    for (auto chunk : str->getChunks()) {
      chunksLE.write(static_cast<uint8_t>(chunk.getKind()));
      chunksLE.write(static_cast<uint8_t>(chunk.getNestingLevel()));
      chunksLE.write(static_cast<uint8_t>(chunk.isAnnotation()));
      if (chunk.hasText()) {
        chunksLE.write(addString(chunk.getText()));
      } else {
        chunksLE.write(static_cast<uint32_t>(~0u));
      }
    }
    return static_cast<uint32_t>(size);
  };

  // RESULTS
  {
    toolchain::support::endian::Writer LE(results, toolchain::endianness::little);
    for (const ContextFreeCodeCompletionResult *R : V.Results) {
      // FIXME: compress bitfield
      LE.write(static_cast<uint8_t>(R->getKind()));
      LE.write(static_cast<uint8_t>(R->getOpaqueAssociatedKind()));
      if (R->isOperator()) {
        LE.write(static_cast<uint8_t>(R->getKnownOperatorKind()));
      } else {
        LE.write(static_cast<uint8_t>(CodeCompletionOperatorKind::None));
      }
      LE.write(static_cast<uint8_t>(R->getMacroRoles().toRaw()));
      LE.write(static_cast<uint8_t>(R->getNotRecommendedReason()));
      LE.write(static_cast<uint8_t>(R->getDiagnosticSeverity()));
      LE.write(static_cast<uint8_t>(R->isSystem()));
      LE.write(static_cast<uint8_t>(R->hasAsyncAlternative()));
      LE.write(
          static_cast<uint32_t>(addCompletionString(R->getCompletionString())));
      LE.write(addString(R->getModuleName()));      // index into strings
      LE.write(addString(R->getBriefDocComment())); // index into strings
      LE.write(addString(R->getDiagnosticMessage())); // index into strings
      LE.write(addString(R->getFilterName())); // index into strings
      LE.write(addString(R->getNameForDiagnostics())); // index into strings

      LE.write(static_cast<uint32_t>(R->getAssociatedUSRs().size()));
      for (unsigned i = 0; i < R->getAssociatedUSRs().size(); ++i) {
        LE.write(addString(R->getAssociatedUSRs()[i]));
      }

      auto resultTypes =
          R->getResultType().getUSRBasedResultTypes(V.USRTypeArena);
      LE.write(static_cast<uint32_t>(resultTypes.size()));
      for (auto resultType : resultTypes) {
        LE.write(addType(resultType)); // index into types
      }
    }
  }
  LE.write(static_cast<uint32_t>(results.tell()));
  out << results.str();

  // CHUNKS
  LE.write(static_cast<uint32_t>(chunks.tell()));
  out << chunks.str();

  // STRINGS
  LE.write(static_cast<uint32_t>(strings.tell()));
  out << strings.str();

  // TYPES
  LE.write(static_cast<uint32_t>(types.tell()));
  out << types.str();
}

/// Get the name for the cached code completion results for a given key \p K in
/// \p cacheDirectory.
///
/// This name is unique (modulo hash collisions) to the key \p K.
static std::string getName(StringRef cacheDirectory,
                           const CodeCompletionCache::Key &K) {
  SmallString<128> name(cacheDirectory);

  // cacheDirectory/ModuleName
  toolchain::sys::path::append(name, K.ModuleName);
  toolchain::raw_svector_ostream OSS(name);

  // name[-with-enabled-options]
  OSS << (K.ResultsHaveLeadingDot ? "-dot" : "")
      << (K.ForTestableLookup ? "-testable" : "")
      << (K.ForPrivateImportLookup ? "-private" : "")
      << (K.AddInitsInToplevel ? "-inits" : "")
      << (K.AddCallWithNoDefaultArgs ? "-nodefaults" : "")
      << (K.Annotated ? "-annotated" : "");
  if (K.SpiGroups.size() > 0) {
    OSS << "-spi";
    for (auto SpiGroup : K.SpiGroups) {
      OSS << "-" << SpiGroup;
    }
  }

  // name[-access-path-components]
  for (StringRef component : K.AccessPath)
    OSS << "-" << component;

  // name-<hash of module filename>
  auto hash = toolchain::hash_value(K.ModuleFilename);
  SmallString<16> hashStr;
  toolchain::APInt(64, uint64_t(hash)).toStringUnsigned(hashStr, /*Radix*/ 36);
  OSS << "-" << hashStr << ".completions";

  return std::string(name.str());
}

std::optional<CodeCompletionCache::ValueRefCntPtr>
OnDiskCodeCompletionCache::get(const Key &K) {
  // Try to find the cached file.
  auto bufferOrErr = toolchain::MemoryBuffer::getFile(getName(cacheDirectory, K));
  if (!bufferOrErr)
    return std::nullopt;

  // Read the cached results, failing if they are out of date.
  auto V = CodeCompletionCache::createValue();
  if (!readCachedModule(bufferOrErr.get().get(), K, *V))
    return std::nullopt;

  return V;
}

std::error_code OnDiskCodeCompletionCache::set(const Key &K, ValueRefCntPtr V) {
  if (K.ModuleFilename.empty())
    return std::make_error_code(std::errc::no_such_file_or_directory);

  // Create the cache directory if it doesn't exist.
  if (auto err = toolchain::sys::fs::create_directories(cacheDirectory))
    return err;

  std::string name = getName(cacheDirectory, K);

  // Create a temporary file to write the results into.
  SmallString<128> tmpName(name + "-%%%%%%");
  int tmpFD;
  if (auto err = toolchain::sys::fs::createUniqueFile(tmpName.str(), tmpFD, tmpName))
    return err;

  // Write the contents of the buffer.
  toolchain::raw_fd_ostream out(tmpFD, /*shouldClose=*/true);
  writeCachedModule(out, K, *V);
  out.flush();
  if (out.has_error())
    return std::make_error_code(std::errc::io_error);

  // Atomically rename the file into its final location.
  return toolchain::sys::fs::rename(tmpName.str(), name);
}

std::optional<CodeCompletionCache::ValueRefCntPtr>
OnDiskCodeCompletionCache::getFromFile(StringRef filename) {
  // Try to find the cached file.
  auto bufferOrErr = toolchain::MemoryBuffer::getFile(filename);
  if (!bufferOrErr)
    return std::nullopt;

  // Make up a key for readCachedModule.
  CodeCompletionCache::Key K{/*ModuleFilename=*/filename.str(),
                             /*ModuleName=*/"<module-name>",
                             /*AccessPath=*/{},
                             /*ResultsHaveLeadingDot=*/false,
                             /*ForTestableLookup=*/false,
                             /*ForPrivateImportLookup=*/false,
                             /*SpiGroups=*/{},
                             /*AddInitsInToplevel=*/false,
                             /*AddCallWithNoDefaultArgs=*/false,
                             /*Annotated=*/false};

  // Read the cached results.
  auto V = CodeCompletionCache::createValue();
  if (!readCachedModule(bufferOrErr.get().get(), K, *V,
                        /*allowOutOfDate*/ true))
    return std::nullopt;

  return V;
}

OnDiskCodeCompletionCache::OnDiskCodeCompletionCache(Twine cacheDirectory)
    : cacheDirectory(cacheDirectory.str()) {}

OnDiskCodeCompletionCache::~OnDiskCodeCompletionCache() {}
