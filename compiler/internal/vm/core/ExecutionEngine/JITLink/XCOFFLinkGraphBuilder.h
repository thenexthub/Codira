//===----- XCOFFLinkGraphBuilder.h - XCOFF LinkGraph builder ----*- C++ -*-===//
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
//
// Generic XCOFF LinkGraph building code.
//
//===----------------------------------------------------------------------===//

#ifndef LIB_EXECUTIONENGINE_JITLINK_XCOFFLINKGRAPHBUILDER_H
#define LIB_EXECUTIONENGINE_JITLINK_XCOFFLINKGRAPHBUILDER_H

#include "vm/core/ExecutionEngine/JITLink/JITLink.h"
#include "vm/core/ExecutionEngine/Orc/SymbolStringPool.h"
#include "vm/core/Object/ObjectFile.h"
#include "vm/core/Object/XCOFFObjectFile.h"
#include "vm/core/TargetParser/SubtargetFeature.h"
#include <memory>

namespace vm::core {
namespace jitlink {

class XCOFFLinkGraphBuilder {
public:
  virtual ~XCOFFLinkGraphBuilder() = default;
  Expected<std::unique_ptr<LinkGraph>> buildGraph();

public:
  XCOFFLinkGraphBuilder(const object::XCOFFObjectFile &Obj,
                        std::shared_ptr<orc::SymbolStringPool> SSP, Triple TT,
                        SubtargetFeatures Features,
                        LinkGraph::GetEdgeKindNameFunction GetEdgeKindName);
  LinkGraph &getGraph() const { return *G; }
  const object::XCOFFObjectFile &getObject() const { return Obj; }

private:
  Error processSections();
  Error processCsectsAndSymbols();
  Error processRelocations();

private:
  const object::XCOFFObjectFile &Obj;
  std::unique_ptr<LinkGraph> G;

  Section *UndefSection;

  struct SectionEntry {
    jitlink::Section *Section;
    object::SectionRef SectionData;
  };

  DenseMap<uint16_t, SectionEntry> SectionTable;
  DenseMap<uint32_t, Block *> CsectTable;
  DenseMap<uint32_t, Symbol *> SymbolIndexTable;
};

} // namespace jitlink
} // namespace vm::core

#endif // LIB_EXECUTIONENGINE_JITLINK_XCOFFLINKGRAPHBUILDER_H
