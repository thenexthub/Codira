//===--- Logging-Mac.mm ---------------------------------------------------===//
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

#if defined(__APPLE__)

#include "Logging_impl.h"
#import <Foundation/Foundation.h>

void IndexStoreDB::Log_impl(const char *loggerName, const char *message) {
  // Using NSLog instead of stderr, to avoid interleaving with other log output
  // in the process. NSLog also logs to asl. Note that we need to print as an
  // NSString here, since printing the C string with '%s' would use the default
  // system encoding instead of UTF-8.
  NSLog(@"%s: %@", loggerName, [NSString stringWithUTF8String:message]);
}

#endif
