//===- DelayedDiagnostic.cpp - Delayed declarator diagnostics -------------===//
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
// This file defines the DelayedDiagnostic class implementation, which
// is used to record diagnostics that are being conditionally produced
// during declarator parsing.
//
// This file also defines AccessedEntity.
//
//===----------------------------------------------------------------------===//

#include "language/Core/Sema/DelayedDiagnostic.h"
#include <cstring>

using namespace language::Core;
using namespace sema;

DelayedDiagnostic
DelayedDiagnostic::makeAvailability(AvailabilityResult AR,
                                    ArrayRef<SourceLocation> Locs,
                                    const NamedDecl *ReferringDecl,
                                    const NamedDecl *OffendingDecl,
                                    const ObjCInterfaceDecl *UnknownObjCClass,
                                    const ObjCPropertyDecl  *ObjCProperty,
                                    StringRef Msg,
                                    bool ObjCPropertyAccess) {
  assert(!Locs.empty());
  DelayedDiagnostic DD;
  DD.Kind = Availability;
  DD.Triggered = false;
  DD.Loc = Locs.front();
  DD.AvailabilityData.ReferringDecl = ReferringDecl;
  DD.AvailabilityData.OffendingDecl = OffendingDecl;
  DD.AvailabilityData.UnknownObjCClass = UnknownObjCClass;
  DD.AvailabilityData.ObjCProperty = ObjCProperty;
  char *MessageData = nullptr;
  if (!Msg.empty()) {
    MessageData = new char [Msg.size()];
    memcpy(MessageData, Msg.data(), Msg.size());
  }
  DD.AvailabilityData.Message = MessageData;
  DD.AvailabilityData.MessageLen = Msg.size();

  DD.AvailabilityData.SelectorLocs = new SourceLocation[Locs.size()];
  memcpy(DD.AvailabilityData.SelectorLocs, Locs.data(),
         sizeof(SourceLocation) * Locs.size());
  DD.AvailabilityData.NumSelectorLocs = Locs.size();

  DD.AvailabilityData.AR = AR;
  DD.AvailabilityData.ObjCPropertyAccess = ObjCPropertyAccess;
  return DD;
}

void DelayedDiagnostic::Destroy() {
  switch (Kind) {
  case Access:
    getAccessData().~AccessedEntity();
    break;

  case Availability:
    delete[] AvailabilityData.Message;
    delete[] AvailabilityData.SelectorLocs;
    break;

  case ForbiddenType:
    break;
  }
}
