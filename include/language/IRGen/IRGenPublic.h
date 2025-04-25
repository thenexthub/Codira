//===--- IRGenPublic.h - Public interface to IRGen --------------*- C++ -*-===//
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
#ifndef SWIFT_IRGEN_IRGENPUBLIC_H
#define SWIFT_IRGEN_IRGENPUBLIC_H

namespace llvm {
  class LLVMContext;
  template<typename T, unsigned N> class SmallVector;
}

namespace language {
class ASTContext;
class LinkLibrary;
class SILModule;

namespace irgen {

class IRGenerator;
class IRGenModule;

/// Create an IRGen module.
std::pair<IRGenerator *, IRGenModule *>
createIRGenModule(SILModule *SILMod, StringRef OutputFilename,
                  StringRef MainInputFilenameForDebugInfo,
                  StringRef PrivateDiscriminator, IRGenOptions &options);

/// Delete the IRGenModule and IRGenerator obtained by the above call.
void deleteIRGenModule(std::pair<IRGenerator *, IRGenModule *> &Module);

/// Gets the ABI version number that'll be set as a flag in the module.
uint32_t getSwiftABIVersion();

} // end namespace irgen
} // end namespace language

#endif
