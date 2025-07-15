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

#ifndef LANGUAGE_LLVMPASSES_PASSESFWD_H
#define LANGUAGE_LLVMPASSES_PASSESFWD_H

namespace toolchain {
  class ModulePass;
  class FunctionPass;
  class ImmutablePass;
  class PassRegistry;

  void initializeCodiraAAWrapperPassPass(PassRegistry &);
  void initializeCodiraARCOptPass(PassRegistry &);
  void initializeCodiraARCContractPass(PassRegistry &);
  void initializeInlineTreePrinterPass(PassRegistry &);
  void initializeLegacyCodiraMergeFunctionsPass(PassRegistry &);
}

namespace language {
  toolchain::FunctionPass *createCodiraARCOptPass();
  toolchain::FunctionPass *createCodiraARCContractPass();
  toolchain::ModulePass *createInlineTreePrinterPass();
  toolchain::ModulePass *createLegacyCodiraMergeFunctionsPass(bool ptrAuthEnabled,
                                                        unsigned ptrAuthKey);
  toolchain::ImmutablePass *createCodiraAAWrapperPass();
} // end namespace language

#endif
