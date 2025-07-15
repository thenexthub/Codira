//===--- Notifications.cpp ------------------------------------------------===//
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

#define DEBUG_TYPE "sil-notifications"

#include "language/SIL/Notifications.h"
#include "language/SIL/SILDefaultOverrideTable.h"
#include "language/SIL/SILMoveOnlyDeinit.h"
#include "toolchain/Support/Debug.h"
#include "toolchain/Support/raw_ostream.h"

using namespace language;

//===----------------------------------------------------------------------===//
// DeserializationNotificationHandler Impl of
// DeserializationNotificationHandlerSet
//===----------------------------------------------------------------------===//

#ifdef DNS_CHAIN_METHOD
#error "DNS_CHAIN_METHOD is defined?!"
#endif
#define DNS_CHAIN_METHOD(Name, FirstTy, SecondTy)                              \
  void DeserializationNotificationHandlerSet::did##Name(FirstTy first,         \
                                                        SecondTy second) {     \
    TOOLCHAIN_DEBUG(toolchain::dbgs()                                                    \
               << "*** Deserialization Notification: " << #Name << " ***\n");  \
    for (auto *p : getRange()) {                                               \
      TOOLCHAIN_DEBUG(toolchain::dbgs()                                                  \
                 << "    Begin Notifying: " << p->getName() << "\n");          \
      p->did##Name(first, second);                                             \
      TOOLCHAIN_DEBUG(toolchain::dbgs()                                                  \
                 << "    End Notifying: " << p->getName() << "\n");            \
    }                                                                          \
    TOOLCHAIN_DEBUG(toolchain::dbgs()                                                    \
               << "*** Completed Deserialization Notifications for " #Name     \
                  "\n");                                                       \
  }
DNS_CHAIN_METHOD(Deserialize, ModuleDecl *, SILFunction *)
DNS_CHAIN_METHOD(DeserializeFunctionBody, ModuleDecl *, SILFunction *)
DNS_CHAIN_METHOD(DeserializeWitnessTableEntries, ModuleDecl *,
                 SILWitnessTable *)
DNS_CHAIN_METHOD(DeserializeDefaultWitnessTableEntries, ModuleDecl *,
                 SILDefaultWitnessTable *)
DNS_CHAIN_METHOD(DeserializeDefaultOverrideTableEntries, ModuleDecl *,
                 SILDefaultOverrideTable *)
DNS_CHAIN_METHOD(Deserialize, ModuleDecl *, SILGlobalVariable *)
DNS_CHAIN_METHOD(Deserialize, ModuleDecl *, SILVTable *)
DNS_CHAIN_METHOD(Deserialize, ModuleDecl *, SILMoveOnlyDeinit *)
DNS_CHAIN_METHOD(Deserialize, ModuleDecl *, SILWitnessTable *)
DNS_CHAIN_METHOD(Deserialize, ModuleDecl *, SILDefaultWitnessTable *)
DNS_CHAIN_METHOD(Deserialize, ModuleDecl *, SILDefaultOverrideTable *)
#undef DNS_CHAIN_METHOD
