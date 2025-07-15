//===--- CompileJobCacheKey.cpp - compile cache key methods ---------------===//
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
// This file contains utility methods for creating compile job cache keys.
//
//===----------------------------------------------------------------------===//

#include "language/Option/Options.h"
#include "toolchain/ADT/STLExtras.h"
#include "toolchain/CAS/CASReference.h"
#include "toolchain/CAS/HierarchicalTreeBuilder.h"
#include "toolchain/CAS/ObjectStore.h"
#include "toolchain/CAS/TreeEntry.h"
#include "toolchain/CAS/TreeSchema.h"
#include "toolchain/Option/ArgList.h"
#include "toolchain/Option/OptTable.h"
#include "toolchain/Support/Endian.h"
#include "toolchain/Support/EndianStream.h"
#include "toolchain/Support/Error.h"
#include "toolchain/Support/MemoryBuffer.h"
#include "toolchain/Support/raw_ostream.h"
#include <toolchain/ADT/SmallString.h>
#include <language/Basic/Version.h>
#include <language/Frontend/CompileJobCacheKey.h>

using namespace language;

// TODO: Rewrite this into CASNodeSchema.
toolchain::Expected<toolchain::cas::ObjectRef> language::createCompileJobBaseCacheKey(
    toolchain::cas::ObjectStore &CAS, ArrayRef<const char *> Args) {
  // Don't count the `-frontend` in the first location since only frontend
  // invocation can have a cache key.
  if (Args.size() > 1 && StringRef(Args.front()) == "-frontend")
    Args = Args.drop_front();

  unsigned MissingIndex;
  unsigned MissingCount;
  std::unique_ptr<toolchain::opt::OptTable> Table = createCodiraOptTable();
  toolchain::opt::InputArgList ParsedArgs = Table->ParseArgs(
      Args, MissingIndex, MissingCount, options::FrontendOption);

  SmallString<256> CommandLine;
  for (auto *Arg : ParsedArgs) {
    const auto &Opt = Arg->getOption();

    // Skip the options that doesn't affect caching.
    if (Opt.hasFlag(options::CacheInvariant))
      continue;

    if (Opt.hasFlag(options::ArgumentIsFileList)) {
      auto FileList = toolchain::MemoryBuffer::getFile(Arg->getValue());
      if (!FileList)
        return toolchain::errorCodeToError(FileList.getError());
      CommandLine.append(Opt.getRenderName());
      CommandLine.push_back(0);
      CommandLine.append((*FileList)->getBuffer());
      CommandLine.push_back(0);
      continue;
    }

    CommandLine.append(Arg->getAsString(ParsedArgs));
    CommandLine.push_back(0);
  }

  toolchain::cas::HierarchicalTreeBuilder Builder;
  auto CMD = CAS.storeFromString(std::nullopt, CommandLine);
  if (!CMD)
    return CMD.takeError();
  Builder.push(*CMD, toolchain::cas::TreeEntry::Regular, "command-line");

  // FIXME: The version is maybe insufficient...
  auto Version =
      CAS.storeFromString(std::nullopt, version::getCodiraFullVersion());
  if (!Version)
    return Version.takeError();
  Builder.push(*Version, toolchain::cas::TreeEntry::Regular, "version");

  if (auto Out = Builder.create(CAS))
    return Out->getRef();
  else
    return Out.takeError();
}

toolchain::Expected<toolchain::cas::ObjectRef>
language::createCompileJobCacheKeyForOutput(toolchain::cas::ObjectStore &CAS,
                                         toolchain::cas::ObjectRef BaseKey,
                                         unsigned InputIndex) {
  std::string InputInfo;
  toolchain::raw_string_ostream OS(InputInfo);

  // CacheKey is the index of the producting input + the base key.
  // Encode the unsigned value as little endian in the field.
  toolchain::support::endian::write<uint32_t>(OS, InputIndex,
                                         toolchain::endianness::little);

  return CAS.storeFromString({BaseKey}, OS.str());
}

toolchain::Error language::printCompileJobCacheKey(toolchain::cas::ObjectStore &CAS,
                                           toolchain::cas::ObjectRef Key,
                                           toolchain::raw_ostream &OS) {
  auto Proxy = CAS.getProxy(Key);
  if (!Proxy)
    return Proxy.takeError();

  if (Proxy->getData().size() != sizeof(uint32_t))
    return toolchain::createStringError("incorrect size for cache key node");
  if (Proxy->getNumReferences() != 1)
    return toolchain::createStringError("incorrect child number for cache key node");

  uint32_t InputIndex = toolchain::support::endian::read<uint32_t>(
      Proxy->getData().data(), toolchain::endianness::little);

  auto Base = Proxy->getReference(0);
  toolchain::cas::TreeSchema Schema(CAS);
  auto Tree = Schema.load(Base);
  if (!Tree)
    return Tree.takeError();

  std::string BaseStr;
  toolchain::raw_string_ostream BaseOS(BaseStr);
  auto Err = Tree->forEachEntry(
      [&](const toolchain::cas::NamedTreeEntry &Entry) -> toolchain::Error {
        auto Ref = Entry.getRef();
        auto DataProxy = CAS.getProxy(Ref);
        if (!DataProxy)
          return DataProxy.takeError();

        BaseOS.indent(2) << Entry.getName() << "\n";
        StringRef Line, Remain = DataProxy->getData();
        while (!Remain.empty()) {
          std::tie(Line, Remain) = Remain.split(0);
          BaseOS.indent(4) << Line << "\n";
        }
        return toolchain::Error::success();
      });
  if (Err)
    return Err;

  toolchain::outs() << "Cache Key " << CAS.getID(Key).toString() << "\n";
  toolchain::outs() << "Codira Compiler Invocation Info:\n";
  toolchain::outs() << BaseStr;
  toolchain::outs() << "Input index: " << InputIndex << "\n";

  return toolchain::Error::success();
}
