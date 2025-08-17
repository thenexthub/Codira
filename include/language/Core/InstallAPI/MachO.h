//===- InstallAPI/MachO.h ---------------------------------------*- C++ -*-===//
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
// Imports and forward declarations for toolchain::MachO types.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_INSTALLAPI_MACHO_H
#define LANGUAGE_CORE_INSTALLAPI_MACHO_H

#include "toolchain/TextAPI/Architecture.h"
#include "toolchain/TextAPI/InterfaceFile.h"
#include "toolchain/TextAPI/PackedVersion.h"
#include "toolchain/TextAPI/Platform.h"
#include "toolchain/TextAPI/RecordVisitor.h"
#include "toolchain/TextAPI/Symbol.h"
#include "toolchain/TextAPI/Target.h"
#include "toolchain/TextAPI/TextAPIWriter.h"
#include "toolchain/TextAPI/Utils.h"

using AliasMap = toolchain::MachO::AliasMap;
using Architecture = toolchain::MachO::Architecture;
using ArchitectureSet = toolchain::MachO::ArchitectureSet;
using SymbolFlags = toolchain::MachO::SymbolFlags;
using RecordLinkage = toolchain::MachO::RecordLinkage;
using Record = toolchain::MachO::Record;
using EncodeKind = toolchain::MachO::EncodeKind;
using GlobalRecord = toolchain::MachO::GlobalRecord;
using InterfaceFile = toolchain::MachO::InterfaceFile;
using ObjCContainerRecord = toolchain::MachO::ObjCContainerRecord;
using ObjCInterfaceRecord = toolchain::MachO::ObjCInterfaceRecord;
using ObjCCategoryRecord = toolchain::MachO::ObjCCategoryRecord;
using ObjCIVarRecord = toolchain::MachO::ObjCIVarRecord;
using ObjCIFSymbolKind = toolchain::MachO::ObjCIFSymbolKind;
using Records = toolchain::MachO::Records;
using RecordLoc = toolchain::MachO::RecordLoc;
using RecordsSlice = toolchain::MachO::RecordsSlice;
using BinaryAttrs = toolchain::MachO::RecordsSlice::BinaryAttrs;
using SymbolSet = toolchain::MachO::SymbolSet;
using SimpleSymbol = toolchain::MachO::SimpleSymbol;
using FileType = toolchain::MachO::FileType;
using PackedVersion = toolchain::MachO::PackedVersion;
using PathSeq = toolchain::MachO::PathSeq;
using PlatformType = toolchain::MachO::PlatformType;
using PathToPlatformSeq = toolchain::MachO::PathToPlatformSeq;
using Target = toolchain::MachO::Target;
using TargetList = toolchain::MachO::TargetList;

#endif // LANGUAGE_CORE_INSTALLAPI_MACHO_H
