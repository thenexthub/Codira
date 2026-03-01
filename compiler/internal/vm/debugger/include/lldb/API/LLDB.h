//===-- LLDB.h --------------------------------------------------*- C++ -*-===//
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

#ifndef LLDB_API_LLDB_H
#define LLDB_API_LLDB_H

#include "lldb/API/SBAddress.h"
#include "lldb/API/SBAddressRange.h"
#include "lldb/API/SBAddressRangeList.h"
#include "lldb/API/SBAttachInfo.h"
#include "lldb/API/SBBlock.h"
#include "lldb/API/SBBreakpoint.h"
#include "lldb/API/SBBreakpointLocation.h"
#include "lldb/API/SBBreakpointName.h"
#include "lldb/API/SBBroadcaster.h"
#include "lldb/API/SBCommandInterpreter.h"
#include "lldb/API/SBCommandInterpreterRunOptions.h"
#include "lldb/API/SBCommandReturnObject.h"
#include "lldb/API/SBCommunication.h"
#include "lldb/API/SBCompileUnit.h"
#include "lldb/API/SBData.h"
#include "lldb/API/SBDebugger.h"
#include "lldb/API/SBDeclaration.h"
#include "lldb/API/SBDefines.h"
#include "lldb/API/SBEnvironment.h"
#include "lldb/API/SBError.h"
#include "lldb/API/SBEvent.h"
#include "lldb/API/SBExecutionContext.h"
#include "lldb/API/SBExpressionOptions.h"
#include "lldb/API/SBFile.h"
#include "lldb/API/SBFileSpec.h"
#include "lldb/API/SBFileSpecList.h"
#include "lldb/API/SBFormat.h"
#include "lldb/API/SBFrame.h"
#include "lldb/API/SBFrameList.h"
#include "lldb/API/SBFunction.h"
#include "lldb/API/SBHostOS.h"
#include "lldb/API/SBInstruction.h"
#include "lldb/API/SBInstructionList.h"
#include "lldb/API/SBLanguageRuntime.h"
#include "lldb/API/SBLanguages.h"
#include "lldb/API/SBLaunchInfo.h"
#include "lldb/API/SBLineEntry.h"
#include "lldb/API/SBListener.h"
#include "lldb/API/SBMemoryRegionInfo.h"
#include "lldb/API/SBMemoryRegionInfoList.h"
#include "lldb/API/SBModule.h"
#include "lldb/API/SBModuleSpec.h"
#include "lldb/API/SBMutex.h"
#include "lldb/API/SBPlatform.h"
#include "lldb/API/SBProcess.h"
#include "lldb/API/SBProcessInfo.h"
#include "lldb/API/SBProcessInfoList.h"
#include "lldb/API/SBProgress.h"
#include "lldb/API/SBQueue.h"
#include "lldb/API/SBQueueItem.h"
#include "lldb/API/SBReproducer.h"
#include "lldb/API/SBSaveCoreOptions.h"
#include "lldb/API/SBSection.h"
#include "lldb/API/SBSourceManager.h"
#include "lldb/API/SBStatisticsOptions.h"
#include "lldb/API/SBStream.h"
#include "lldb/API/SBStringList.h"
#include "lldb/API/SBStructuredData.h"
#include "lldb/API/SBSymbol.h"
#include "lldb/API/SBSymbolContext.h"
#include "lldb/API/SBSymbolContextList.h"
#include "lldb/API/SBTarget.h"
#include "lldb/API/SBThread.h"
#include "lldb/API/SBThreadCollection.h"
#include "lldb/API/SBThreadPlan.h"
#include "lldb/API/SBTrace.h"
#include "lldb/API/SBType.h"
#include "lldb/API/SBTypeCategory.h"
#include "lldb/API/SBTypeEnumMember.h"
#include "lldb/API/SBTypeFilter.h"
#include "lldb/API/SBTypeFormat.h"
#include "lldb/API/SBTypeNameSpecifier.h"
#include "lldb/API/SBTypeSummary.h"
#include "lldb/API/SBTypeSynthetic.h"
#include "lldb/API/SBUnixSignals.h"
#include "lldb/API/SBValue.h"
#include "lldb/API/SBValueList.h"
#include "lldb/API/SBVariablesOptions.h"
#include "lldb/API/SBWatchpoint.h"

#endif // LLDB_API_LLDB_H
