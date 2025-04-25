//===--- PassesFwd.h - Creation functions for LLVM passes -------*- C++ -*-===//
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

#ifndef SWIFT_LLVMPASSES_PASSESFWD_H
#define SWIFT_LLVMPASSES_PASSESFWD_H

namespace llvm {
  class ModulePass;
  class FunctionPass;
  class ImmutablePass;
  class PassRegistry;

  void initializeSwiftAAWrapperPassPass(PassRegistry &);
  void initializeSwiftARCOptPass(PassRegistry &);
  void initializeSwiftARCContractPass(PassRegistry &);
  void initializeInlineTreePrinterPass(PassRegistry &);
  void initializeLegacySwiftMergeFunctionsPass(PassRegistry &);
}

namespace language {
  llvm::FunctionPass *createSwiftARCOptPass();
  llvm::FunctionPass *createSwiftARCContractPass();
  llvm::ModulePass *createInlineTreePrinterPass();
  llvm::ModulePass *createLegacySwiftMergeFunctionsPass(bool ptrAuthEnabled,
                                                        unsigned ptrAuthKey);
  llvm::ImmutablePass *createSwiftAAWrapperPass();
} // end namespace language

#endif
