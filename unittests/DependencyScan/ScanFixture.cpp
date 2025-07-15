//===-------- ScanFixture.cpp - Dependency scanning tests -*- C++ -------*-===//
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

#include "ScanFixture.h"
#include "language/Basic/ToolchainInitializer.h"
#include "toolchain/Support/FileSystem.h"
#include "gtest/gtest.h"

using namespace language;
using namespace language::unittest;
using namespace language::dependencies;

ScanTest::ScanTest() : ScannerTool(), StdLibDir(LANGUAGELIB_DIR) {
  INITIALIZE_LLVM();
  // Create a temporary filesystem workspace for this test.
  toolchain::sys::fs::createUniqueDirectory("ScanTest.Workspace",
                                       TemporaryTestWorkspace);
}

ScanTest::~ScanTest() {
  toolchain::sys::fs::remove_directories(TemporaryTestWorkspace);
}
