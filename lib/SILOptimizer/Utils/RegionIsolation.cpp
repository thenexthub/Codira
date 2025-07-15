//===--- RegionIsolation.cpp ----------------------------------------------===//
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

#include "language/SILOptimizer/Utils/RegionIsolation.h"

#include "toolchain/Support/CommandLine.h"

using namespace language;
using namespace regionisolation;

//===----------------------------------------------------------------------===//
//                               MARK: Logging
//===----------------------------------------------------------------------===//

LoggingFlag language::regionisolation::ENABLE_LOGGING;

static toolchain::cl::opt<LoggingFlag, true> // The parser
    RegionBasedIsolationLog(
        "sil-regionbasedisolation-log",
        toolchain::cl::desc("Enable logging for SIL region-based isolation "
                       "diagnostics"),
        toolchain::cl::Hidden,
        toolchain::cl::values(
            clEnumValN(LoggingFlag::Off, "none", "no logging"),
            clEnumValN(LoggingFlag::Normal, "on", "non verbose logging"),
            clEnumValN(LoggingFlag::Verbose, "verbose", "verbose logging")),
        toolchain::cl::location(language::regionisolation::ENABLE_LOGGING));
