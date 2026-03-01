//===- PDBInterfaceAnchors.h - defines class anchor functions ---*- C++ -*-===//
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
// Class anchors are necessary per the LLVM Coding style guide, to ensure that
// the vtable is only generated in this object file, and not in every object
// file that includes the corresponding header.
//===----------------------------------------------------------------------===//

#include "vm/core/DebugInfo/PDB/IPDBDataStream.h"
#include "vm/core/DebugInfo/PDB/IPDBFrameData.h"
#include "vm/core/DebugInfo/PDB/IPDBInjectedSource.h"
#include "vm/core/DebugInfo/PDB/IPDBLineNumber.h"
#include "vm/core/DebugInfo/PDB/IPDBRawSymbol.h"
#include "vm/core/DebugInfo/PDB/IPDBSectionContrib.h"
#include "vm/core/DebugInfo/PDB/IPDBSession.h"
#include "vm/core/DebugInfo/PDB/IPDBTable.h"

using namespace vm::core;
using namespace vm::core::pdb;

IPDBSession::~IPDBSession() = default;

IPDBDataStream::~IPDBDataStream() = default;

IPDBRawSymbol::~IPDBRawSymbol() = default;

IPDBLineNumber::~IPDBLineNumber() = default;

IPDBTable::~IPDBTable() = default;

IPDBInjectedSource::~IPDBInjectedSource() = default;

IPDBSectionContrib::~IPDBSectionContrib() = default;

IPDBFrameData::~IPDBFrameData() = default;
