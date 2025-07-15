//===--- RegionIsolation.h ------------------------------------------------===//
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
///
/// \file This file just declares some command line options that are used for
/// the various parts of region analysis based diagnostics.
///
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_SILOPTIMIZER_UTILS_REGIONANALYSIS_H
#define LANGUAGE_SILOPTIMIZER_UTILS_REGIONANALYSIS_H

namespace language {

namespace regionisolation {

enum class LoggingFlag {
  Off = 0,
  Normal = 0x1,
  Verbose = 0x3,
};

extern LoggingFlag ENABLE_LOGGING;

#ifdef REGIONBASEDISOLATION_LOG
#error "REGIONBASEDISOLATION_LOG already defined?!"
#endif

#ifdef REGIONBASEDISOLATION_VERBOSE_LOG
#error "REGIONBASEDISOLATION_VERBOSE_LOG already defined?!"
#endif

#define REGIONBASEDISOLATION_LOG(...)                                          \
  do {                                                                         \
    if (language::regionisolation::ENABLE_LOGGING !=                              \
        language::regionisolation::LoggingFlag::Off) {                            \
      __VA_ARGS__;                                                             \
    }                                                                          \
  } while (0);

#define REGIONBASEDISOLATION_VERBOSE_LOG(...)                                  \
  do {                                                                         \
    if (language::regionisolation::ENABLE_LOGGING ==                              \
        language::regionisolation::LoggingFlag::Verbose) {                        \
      __VA_ARGS__;                                                             \
    }                                                                          \
  } while (0);

} // namespace regionisolation

} // namespace language

#endif
